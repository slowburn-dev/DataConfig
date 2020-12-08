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

FDcReadStateClass& PushClassPropertyState(FDcPropertyReader* Reader, UObject* InClassObject, UClass* InClass, FDcReadStateClass::EType InType, const FName& InObjectName)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateClass>(GetTopStorage(Reader), InClassObject, InClass, InType, InObjectName);
}

FDcReadStateStruct& PushStructPropertyState(FDcPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass, const FName& InStructName)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass, InStructName);
}

FDcReadStateMap& PushMappingPropertyState(FDcPropertyReader* Reader, void* InMapPtr, FMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

FDcReadStateArray& PushArrayPropertyState(FDcPropertyReader* Reader, void* InArrayPtr, FArrayProperty* InArrayProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateArray>(GetTopStorage(Reader), InArrayPtr, InArrayProperty);
}

FDcReadStateSet& PushSetPropertyState(FDcPropertyReader* Reader, void* InSetPtr, FSetProperty* InSetProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FDcReadStateSet>(GetTopStorage(Reader), InSetPtr, InSetProperty);
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

template<typename TScalar>
FORCEINLINE FDcResult ReadTopStateScalarProperty(FDcPropertyReader* Self, TScalar* OutPtr)
{
	using TProperty = TPropertyTypeMap<TScalar>::Type;

	FScopedStackedReader StackedReader(Self);

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(Self).ReadDataEntry(TProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		ReadPropertyValueConversion<TProperty, TScalar>(Datum.Property, Datum.DataPtr, OutPtr);
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
		PushClassPropertyState(
			this,
			Obj,
			Datum.CastChecked<UClass>(),
			FDcReadStateClass::EType::Root,
			Obj->GetFName()
		);
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

bool FDcPropertyReader::Coercion(EDcDataEntry ToEntry)
{
	EDcDataEntry Next;
	if (!PeekRead(&Next).Ok())
		return false;

	if (Next == EDcDataEntry::ArrayRoot)
	{
		return ToEntry == EDcDataEntry::Blob;
	}
	else if (Next == EDcDataEntry::StructRoot)
	{
		return ToEntry == EDcDataEntry::Blob;
	}
	else
	{
		return false;
	}
}

FDcResult FDcPropertyReader::PeekRead(EDcDataEntry* OutPtr)
{
	FScopedStackedReader StackedReader(this);
	return GetTopState(this).PeekRead(OutPtr);
}

FDcResult FDcPropertyReader::ReadBool(bool* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadString(FString* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadText(FText* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadEnum(FDcEnumData* OutPtr)
{
	FScopedStackedReader StackedReader(this);

	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(FEnumProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		FEnumProperty* EnumProperty = Datum.CastChecked<FEnumProperty>();
		//	atm UENum API suports int32/int64 so use int64 for all enum for now
		if (EnumProperty->GetUnderlyingProperty()->IsA<FUInt64Property>())
			return DC_FAIL(DcDReadWrite, UInt64EnumNotSupported) << FormatHighlight();

		UEnum* Enum = EnumProperty->GetEnum();
		OutPtr->Type = Enum->GetFName();
		OutPtr->Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Datum.DataPtr);
		OutPtr->Name = Enum->GetNameByValue(OutPtr->Value);
	}

	return DcOk();
}

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
		DC_TRY(TopState.ReadDataEntry(FStructProperty::StaticClass(), Datum));

		FDcReadStateStruct& ChildStruct = PushStructPropertyState(
			this,
			Datum.DataPtr,
			Datum.CastChecked<FStructProperty>()->Struct,
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

FDcResult FDcPropertyReader::ReadClassRoot(FDcObjectPropertyStat* OutClassPtr)
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
		DC_TRY(TopState.ReadDataEntry(FObjectProperty::StaticClass(), Datum));

		FObjectProperty* ObjProperty = Datum.CastChecked<FObjectProperty>();
		check(ObjProperty);
		FDcReadStateClass& ChildClass = PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			ObjProperty->HasAnyPropertyFlags(CPF_InstancedReference)
				? FDcReadStateClass::EType::PropertyInstanced
				: FDcReadStateClass::EType::PropertyNormal,
			ObjProperty->GetFName()
		);
		DC_TRY(ChildClass.ReadClassRoot(OutClassPtr));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadClassEnd(FDcObjectPropertyStat* OutClassPtr)
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
			//	TODO this might never get called, figure it out
			checkNoEntry();
			DC_TRY(MapState->ReadMapRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(FMapProperty::StaticClass(), Datum));

		FDcReadStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<FMapProperty>());
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
			//	TODO this might never get called, figure it out
			checkNoEntry();
			DC_TRY(ArrayState->ReadArrayRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(FArrayProperty::StaticClass(), Datum));

		FDcReadStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<FArrayProperty>());
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

FDcResult FDcPropertyReader::ReadSetRoot()
{
	FScopedStackedReader StackedReader(this);

	FDcBaseReadState& TopState = GetTopState(this);

	{
		//	TODO this might never get called, figure it out
		FDcReadStateSet* SetState = TopState.As<FDcReadStateSet>();
		if (SetState != nullptr
			&& SetState->State == FDcReadStateSet::EState::ExpectRoot)
		{
			DC_TRY(SetState->ReadSetRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(FSetProperty::StaticClass(), Datum));

		FDcReadStateSet& ChildSet = PushSetPropertyState(this, Datum.DataPtr, Datum.CastChecked<FSetProperty>());
		DC_TRY(ChildSet.ReadSetRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadSetEnd()
{
	FScopedStackedReader StackedReader(this);

	if (FDcReadStateSet* SetState = TryGetTopState<FDcReadStateSet>(this))
	{
		DC_TRY(SetState->ReadSetEnd());
		PopState<FDcReadStateSet>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateArray::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadObjectReference(UObject** OutPtr)
{
	FScopedStackedReader StackedReader(this);

	//	only class property reads reference
	if (FDcReadStateClass* ClassState = TryGetTopState<FDcReadStateClass>(this))
	{
		return ClassState->ReadObjectReference(OutPtr);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcReadStateClass::ID << (int)GetTopState(this).GetType();

	}
}

FDcResult FDcPropertyReader::ReadClassReference(UClass** OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadWeakObjectReference(FWeakObjectPtr* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadLazyObjectReference(FLazyObjectPtr* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadSoftObjectReference(FSoftObjectPath* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadSoftClassReference(FSoftClassPath* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInterfaceReference(FScriptInterface* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadDelegate(FScriptDelegate* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadMulticastInlineDelegate(FMulticastScriptDelegate* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadMulticastSparseDelegate(FSparseDelegate* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadInt8(int8* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt16(int16* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt32(int32* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadInt64(int64* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadUInt8(uint8* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt16(uint16* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt32(uint32* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadUInt64(uint64* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadFloat(float* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }
FDcResult FDcPropertyReader::ReadDouble(double* OutPtr) { return ReadTopStateScalarProperty(this, OutPtr); }

FDcResult FDcPropertyReader::ReadBlob(FDcBlobViewData* OutPtr)
{
	FScopedStackedReader StackedReader(this);

	EDcDataEntry Next;
	DC_TRY(GetTopState(this).PeekRead(&Next));

	if (Next == EDcDataEntry::ArrayRoot)
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(FArrayProperty::StaticClass(), Datum));

		FArrayProperty* ArrayProperty = Datum.CastChecked<FArrayProperty>();
		FScriptArrayHelper ScriptArray(ArrayProperty, Datum.DataPtr);

		if (OutPtr)
		{
			*OutPtr = {
				(uint8*)ScriptArray.GetRawPtr(),
				ScriptArray.Num() * ArrayProperty->Inner->ElementSize
			};
		}

		return DcOk();
	}
	else if (Next == EDcDataEntry::StructRoot)
	{
		FDcPropertyDatum Datum;
		DC_TRY(GetTopState(this).ReadDataEntry(FStructProperty::StaticClass(), Datum));

		FStructProperty* StructProperty = Datum.CastChecked<FStructProperty>();
		if (OutPtr)
		{
			*OutPtr = {
				(uint8*)Datum.DataPtr,
				StructProperty->ElementSize
			};
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< EDcDataEntry::ArrayRoot << Next;
	}
}

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

	bool bLastIsContainer = false;
	int Num = States.Num();
	for (int Ix = 1; Ix < Num; Ix++)
	{
		FDcBaseReadState& ReadState = AsReadState(&States[Ix].ImplStorage);
		DcPropertyHighlight::EFormatSeg Seg;
		if (bLastIsContainer)
			Seg = DcPropertyHighlight::EFormatSeg::ParentIsContainer;
		else if (Ix == Num - 1)
			Seg = DcPropertyHighlight::EFormatSeg::Last;
		else
			Seg = DcPropertyHighlight::EFormatSeg::Normal;

		ReadState.FormatHighlightSegment(Segments, Seg);

		EDcPropertyReadType StateType = ReadState.GetType();
		bLastIsContainer = StateType == EDcPropertyReadType::ArrayProperty
			|| StateType == EDcPropertyReadType::SetProperty
			|| StateType == EDcPropertyReadType::MapProperty;
	}

	FString Path = FString::Join(Segments, TEXT("."));
	OutHighlight.Formatted = FString::Printf(TEXT("Reading property: %s"), *Path);
	return OutHighlight;
}

void FDcPropertyReader::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Diag << FormatHighlight();
}

