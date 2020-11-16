#include "DataConfig/Property/DcPropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"

//	need these as readers needs to push states

static FORCEINLINE FDcPropertyReader::FPropertyState::ImplStorageType* GetTopStorage(FDcPropertyReader* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FDcBaseReadState& AsReadState(FDcPropertyReader::FPropertyState::ImplStorageType* Storage)
{
	return *reinterpret_cast<FDcBaseReadState*>(Storage);
}

static FORCEINLINE FDcBaseReadState& GetTopState(FDcPropertyReader* Self)
{
	return AsReadState(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FDcPropertyReader* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FDcPropertyReader* Self) {
	return GetTopState(Self).As<TState>();
}

FDcReadStateNil& PushNilState(FDcPropertyReader* Reader)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateNil>(GetTopStorage(Reader));
}

FDcReadStateClass& PushClassPropertyState(FDcPropertyReader* Reader, UObject* InClassObject, UClass* InClass, FDcReadStateClass::EType InType)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateClass>(GetTopStorage(Reader), InClassObject, InClass, InType);
}

FDcReadStateStruct& PushStructPropertyState(FDcPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass, const FName& InStructName)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass, InStructName);
}

FDcReadStateMap& PushMappingPropertyState(FDcPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

FDcReadStateArray& PushArrayPropertyState(FDcPropertyReader* Reader, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateArray>(GetTopStorage(Reader), InArrayPtr, InArrayProperty);
}

void PopState(FDcPropertyReader* Reader)
{
	Reader->States.Pop();
	check(Reader->States.Num() >= 1);
}


template<typename TState>
static void PopState(FDcPropertyReader* Reader)
{
	check(TState::ID == GetTopState(Reader).GetType());
	PopState(Reader);
}


template<typename TPrimitive>
FORCEINLINE FDcResult ReadTopStateProperty(FDcPropertyReader* Self, TPrimitive* OutPtr)
{
	using TProperty = TPropertyTypeMap<TPrimitive>::Type;

	FScopedStackedReader StackedReader(Self);

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(Self).ReadDataEntry(TProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		*OutPtr = Datum.CastChecked<TProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcPropertyReader::FDcPropertyReader()
{
	PushNilState(this);
}

FDcPropertyReader::FDcPropertyReader(FDcPropertyDatum Datum)
	: FDcPropertyReader()
{
	if (Datum.IsNone())
	{
		//	pass
	}
	else if (Datum.Property->IsA<UClass>())
	{
		UObject* Obj = (UObject*)(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassPropertyState(this, Obj, Datum.CastChecked<UClass>(), FDcReadStateClass::EType::Root);
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructPropertyState(this, Datum.DataPtr, CastChecked<UScriptStruct>(Datum.Property), FName(TEXT("$root")));
	}
	else
	{
		checkNoEntry();
	}
}

FDcResult FDcPropertyReader::ReadNext(EDcDataEntry* OutPtr)
{
	FScopedStackedReader StackedReader(this);
	return GetTopState(this).PeekRead(OutPtr);
}

FDcResult FDcPropertyReader::ReadBool(bool* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadString(FString* OutPtr) { return ReadTopStateProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadName(FName* OutPtr)
{
	FScopedStackedReader StackedReader(this);

	DC_TRY(GetTopState(this).ReadName(OutPtr));

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructRoot(FName* OutNamePtr)
{
	FScopedStackedReader StackedReader(this);

	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateStruct* StructState = TopState.As<FDcReadStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FDcReadStateStruct::EState::ExpectRoot)
		{
			DC_TRY(StructState->ReadStructRoot(OutNamePtr));
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UStructProperty::StaticClass(), Datum));

		FDcReadStateStruct& ChildStruct = PushStructPropertyState(
			this,
			Datum.DataPtr,
			Datum.CastChecked<UStructProperty>()->Struct,
			Datum.Property->GetFName()
		);
		DC_TRY(ChildStruct.ReadStructRoot(OutNamePtr));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructEnd(FName* OutNamePtr)
{
	FScopedStackedReader StackedReader(this);

	if (FDcReadStateStruct* StructState = TryGetTopState<FDcReadStateStruct>(this))
	{
		DC_TRY(StructState->ReadStructEnd(OutNamePtr));
		PopState<FDcReadStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr)
{
	FScopedStackedReader StackedReader(this);

	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateClass* ClassState = TopState.As<FDcReadStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FDcReadStateClass::EState::ExpectRoot)
		{
			DC_TRY(ClassState->ReadClassRoot(OutClassPtr));
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UObjectProperty::StaticClass(), Datum));

		UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
		check(ObjProperty);
		FDcReadStateClass& ChildClass = PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			ObjProperty->HasAnyPropertyFlags(CPF_InstancedReference)
				? FDcReadStateClass::EType::PropertyInstanced
				: FDcReadStateClass::EType::PropertyNormal
		);
		DC_TRY(ChildClass.ReadClassRoot(OutClassPtr));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr)
{
	FScopedStackedReader StackedReader(this);

	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		DC_TRY(ClassState->ReadClassEnd(OutClassPtr));
		PopState<FDcReadStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadMapRoot()
{
	FScopedStackedReader StackedReader(this);

	FDcBaseReadState& TopState = GetTopState(this);
	{
		FDcReadStateMap* MapState = TopState.As<FDcReadStateMap>();
		if (MapState != nullptr
			&& MapState->State == FDcReadStateMap::EState::ExpectRoot)
		{
			DC_TRY(MapState->ReadMapRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UMapProperty::StaticClass(), Datum));

		FDcReadStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		DC_TRY(ChildMap.ReadMapRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadMapEnd()
{
	FScopedStackedReader StackedReader(this);

	if (FDcReadStateMap* StateMap = TryGetTopState<FDcReadStateMap>(this))
	{
		DC_TRY(StateMap->ReadMapEnd());
		PopState<FDcReadStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadArrayRoot()
{
	FScopedStackedReader StackedReader(this);

	FDcBaseReadState& TopState = GetTopState(this);

	{
		FDcReadStateArray* ArrayState = TopState.As<FDcReadStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FDcReadStateArray::EState::ExpectRoot)
		{
			DC_TRY(ArrayState->ReadArrayRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UArrayProperty::StaticClass(), Datum));

		FDcReadStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		DC_TRY(ChildArray.ReadArrayRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadArrayEnd()
{
	FScopedStackedReader StackedReader(this);

	if (FDcReadStateArray* ArrayState = TryGetTopState<FDcReadStateArray>(this))
	{
		DC_TRY(ArrayState->ReadArrayEnd());
		PopState<FDcReadStateArray>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateArray::ID << (int)GetTopState(this).GetType();

	}
}


FDcResult FDcPropertyReader::ReadReference(UObject** OutPtr)
{
	FScopedStackedReader StackedReader(this);

	//	only class property reads reference
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		return ClassState->ReadReference(OutPtr);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();

	}
}

FDcResult FDcPropertyReader::ReadInt8(int8* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt16(int16* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt32(int32* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt64(int64* OutPtr) { return ReadTopStateProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadUInt8(uint8* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt16(uint16* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt32(uint32* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt64(uint64* OutPtr) { return ReadTopStateProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadFloat(float* OutPtr) { return ReadTopStateProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadDouble(double* OutPtr) { return ReadTopStateProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadNil()
{
	FScopedStackedReader StackedReader(this);

	//	only class property accepts nil
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		return ClassState->ReadNil();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcDiagnosticHighlight FDcPropertyReader::FormatHighlight()
{
	FDcDiagnosticHighlight OutHighlight;
	TArray<FString> Segments;

	int Num = States.Num();
	for (int Ix = 1; Ix < Num; Ix++)
		AsReadState(&States[Ix].ImplStorage).FormatHighlightSegment(Segments,
			Ix == Num - 1 
			? FDcBaseReadState::EFormatSeg::Last
			: FDcBaseReadState::EFormatSeg::Normal
		);

	OutHighlight.Formatted = FString::Join(Segments, TEXT("."));
	return OutHighlight;
}

