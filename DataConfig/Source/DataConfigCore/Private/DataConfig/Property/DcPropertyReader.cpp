#include "DataConfig/Property/DcPropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"

//	need these as readers needs to push states
using ReaderStorageType = FDcPropertyReader::FPropertyState::ImplStorageType;

static FORCEINLINE ReaderStorageType* GetTopStorage(FDcPropertyReader* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FBaseReadState& GetTopState(FDcPropertyReader* Self)
{
	return *reinterpret_cast<FBaseReadState*>(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FDcPropertyReader* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FDcPropertyReader* Self) {
	return GetTopState(Self).As<TState>();
}

FReadStateNil& PushNilState(FDcPropertyReader* Reader)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateNil>(GetTopStorage(Reader));
}

FReadStateClass& PushClassPropertyState(FDcPropertyReader* Reader, UObject* InClassObject, UClass* InClass, FReadStateClass::EType InType)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateClass>(GetTopStorage(Reader), InClassObject, InClass, InType);
}

FReadStateStruct& PushStructPropertyState(FDcPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass);
}

FReadStateMap& PushMappingPropertyState(FDcPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

FReadStateArray& PushArrayPropertyState(FDcPropertyReader* Reader, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateArray>(GetTopStorage(Reader), InArrayPtr, InArrayProperty);
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


template<typename TProperty, typename TPrimitive> FORCEINLINE FDcResult ReadTopStateProperty(FDcPropertyReader* Self, TPrimitive* OutPtr)
{
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
		PushClassPropertyState(this, Obj, Datum.CastChecked<UClass>(), FReadStateClass::EType::Root);
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructPropertyState(this, Datum.DataPtr, CastChecked<UScriptStruct>(Datum.Property));
	}
	else
	{
		checkNoEntry();
	}
}

FDcResult FDcPropertyReader::ReadNext(EDcDataEntry* OutPtr)
{
	return GetTopState(this).PeekRead(OutPtr);
}

FDcResult FDcPropertyReader::ReadBool(bool* OutPtr)
{
	return ReadTopStateProperty<UBoolProperty, bool>(this, OutPtr);
}

FDcResult FDcPropertyReader::ReadName(FName* OutPtr)
{
	DC_TRY(GetTopState(this).ReadName(OutPtr));

	return DcOk();
}

FDcResult FDcPropertyReader::ReadString(FString* OutPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(GetTopState(this).ReadDataEntry(UStrProperty::StaticClass(), Datum));

	if (OutPtr)
	{
		*OutPtr = Datum.CastChecked<UStrProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructRoot(FName* OutNamePtr)
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateStruct* StructState = TopState.As<FReadStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FReadStateStruct::EState::ExpectRoot)
		{
			DC_TRY(StructState->ReadStructRoot(OutNamePtr));
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UStructProperty::StaticClass(), Datum));

		FReadStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.CastChecked<UStructProperty>()->Struct);
		DC_TRY(ChildStruct.ReadStructRoot(OutNamePtr));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadStructEnd(FName* OutNamePtr)
{
	if (FReadStateStruct* StructState = TryGetTopState<FReadStateStruct>(this))
	{
		DC_TRY(StructState->ReadStructEnd(OutNamePtr));
		PopState<FReadStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr)
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateClass* ClassState = TopState.As<FReadStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FReadStateClass::EState::ExpectRoot)
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
		FReadStateClass& ChildClass = PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			ObjProperty->HasAnyPropertyFlags(CPF_InstancedReference)
				? FReadStateClass::EType::PropertyInstanced
				: FReadStateClass::EType::PropertyNormal
		);
		DC_TRY(ChildClass.ReadClassRoot(OutClassPtr));
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr)
{
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		DC_TRY(ClassState->ReadClassEnd(OutClassPtr));
		PopState<FReadStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadMapRoot()
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateMap* MapState = TopState.As<FReadStateMap>();
		if (MapState != nullptr
			&& MapState->State == FReadStateMap::EState::ExpectRoot)
		{
			DC_TRY(MapState->ReadMapRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UMapProperty::StaticClass(), Datum));

		FReadStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		DC_TRY(ChildMap.ReadMapRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadMapEnd()
{
	if (FReadStateMap* StateMap = TryGetTopState<FReadStateMap>(this))
	{
		DC_TRY(StateMap->ReadMapEnd());
		PopState<FReadStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyReader::ReadArrayRoot()
{
	FBaseReadState& TopState = GetTopState(this);

	{
		FReadStateArray* ArrayState = TopState.As<FReadStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FReadStateArray::EState::ExpectRoot)
		{
			DC_TRY(ArrayState->ReadArrayRoot());
			return DcOk();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.ReadDataEntry(UArrayProperty::StaticClass(), Datum));

		FReadStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		DC_TRY(ChildArray.ReadArrayRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyReader::ReadArrayEnd()
{
	if (FReadStateArray* ArrayState = TryGetTopState<FReadStateArray>(this))
	{
		DC_TRY(ArrayState->ReadArrayEnd());
		PopState<FReadStateArray>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateArray::ID << (int)GetTopState(this).GetType();

	}
}


FDcResult FDcPropertyReader::ReadReference(UObject** OutPtr)
{
	//	only class property reads reference
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		return ClassState->ReadReference(OutPtr);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();

	}
}

FDcResult FDcPropertyReader::ReadNil()
{
	//	only class property accepts nil
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		return ClassState->ReadNil();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

