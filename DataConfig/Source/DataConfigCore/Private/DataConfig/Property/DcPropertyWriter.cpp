#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyWriteStates.h"

static FORCEINLINE FDcBaseWriteState& GetTopState(FDcPropertyWriter* Self)
{
	return *reinterpret_cast<FDcBaseWriteState*>(&Self->States.Top().ImplStorage);
}

using WriterStorageType = FDcPropertyWriter::FPropertyState::ImplStorageType;
static FORCEINLINE WriterStorageType* GetTopStorage(FDcPropertyWriter* Self)
{
	return &Self->States.Top().ImplStorage;
}

template<typename TState>
static TState& GetTopState(FDcPropertyWriter* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FDcPropertyWriter* Self) {
	return GetTopState(Self).As<TState>();
}

static FDcWriteStateNil& PushNilState(FDcPropertyWriter* Writer) {
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateNil>(GetTopStorage(Writer));
}

static FDcWriteStateClass& PushClassRootState(FDcPropertyWriter* Writer, UObject* InClassObject, UClass* InClass)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InClassObject, InClass);
}

static FDcWriteStateClass& PushClassPropertyState(FDcPropertyWriter* Writer, void* InDataPtr, UObjectProperty* InObjProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateClass>(GetTopStorage(Writer), InDataPtr, InObjProperty);
}

static FDcWriteStateStruct& PushStructPropertyState(FDcPropertyWriter* Writer, void* InStructPtr, UScriptStruct* InStructStruct)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateStruct>(GetTopStorage(Writer), InStructPtr, InStructStruct);
}

static FDcWriteStateMap& PushMappingPropertyState(FDcPropertyWriter* Writer, void* InMapPtr, UMapProperty* InMapProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateMap>(GetTopStorage(Writer), InMapPtr, InMapProperty);
}

static FDcWriteStateArray& PushArrayPropertyState(FDcPropertyWriter* Writer, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FDcWriteStateArray>(GetTopStorage(Writer), InArrayPtr, InArrayProperty);
}

static void PopState(FDcPropertyWriter* Writer)
{
	Writer->States.Pop();
	check(Writer->States.Num() >= 1);
}

template<typename TState>
static void PopState(FDcPropertyWriter* Writer)
{
	check(TState::ID == GetTopState(Writer).GetType());
	PopState(Writer);
}

FDcPropertyWriter::FDcPropertyWriter()
{
	PushNilState(this);
}

FDcPropertyWriter::FDcPropertyWriter(FDcPropertyDatum Datum)
	: FDcPropertyWriter()
{
	if (Datum.IsNone())
	{
		//	pass
	}
	else if (Datum.Property->IsA<UClass>())
	{
		UObject* Obj = reinterpret_cast<UObject*>(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassRootState(this, Obj, Datum.CastChecked<UClass>());
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructPropertyState(this,
			Datum.DataPtr,
			CastChecked<UScriptStruct>(Datum.Property)
		);
	}
	else
	{
		checkNoEntry();
	}
}

FDcResult FDcPropertyWriter::WriteNext(EDcDataEntry Next)
{
	return GetTopState(this).Peek(Next);
}

FDcResult FDcPropertyWriter::WriteBool(bool Value)
{
	return WriteValue<UBoolProperty, bool>(GetTopState(this), Value);
}

FDcResult FDcPropertyWriter::WriteName(const FName& Value)
{
	return GetTopState(this).WriteName(Value);
}

FDcResult FDcPropertyWriter::WriteString(const FString& Value)
{
	return WriteValue<UStrProperty, FString>(GetTopState(this), Value);
}

FDcResult FDcPropertyWriter::WriteStructRoot(const FName& Name)
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateStruct* StructState = TopState.As<FDcWriteStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FDcWriteStateStruct::EState::ExpectRoot)
		{
			return StructState->WriteStructRoot(Name);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UStructProperty::StaticClass(), Datum));

		FDcWriteStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.CastChecked<UStructProperty>()->Struct);
		DC_TRY(ChildStruct.WriteStructRoot(Name));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructEnd(const FName& Name)
{
	if (FDcWriteStateStruct* StructState = TryGetTopState<FDcWriteStateStruct>(this))
	{
		DC_TRY(StructState->WriteStructEnd(Name));
		PopState<FDcWriteStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

void FDcPropertyWriter::PushTopClassPropertyState(FDcPropertyDatum& Datum)
{
	UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
	check(ObjProperty);
	PushClassPropertyState(this, Datum.DataPtr, ObjProperty);
}


FDcResult FDcPropertyWriter::WriteClassRoot(const FDcClassPropertyStat& Class)
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateClass* ClassState = TopState.As<FDcWriteStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FDcWriteStateClass::EState::ExpectRoot)
		{
			return ClassState->WriteClassRoot(Class);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UObjectProperty::StaticClass(), Datum));

		PushTopClassPropertyState(Datum);
		DC_TRY(GetTopState(this).As<FDcWriteStateClass>()->WriteClassRoot(Class));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteClassEnd(const FDcClassPropertyStat& Class)
{
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		DC_TRY(ClassState->WriteClassEnd(Class));
		PopState<FDcWriteStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteMapRoot()
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateMap* MapState = TopState.As<FDcWriteStateMap>();
		if (MapState != nullptr
			&& MapState->State == FDcWriteStateMap::EState::ExpectRoot)
		{
			return MapState->WriteMapRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UMapProperty::StaticClass(), Datum));

		FDcWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		DC_TRY(ChildMap.WriteMapRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteMapEnd()
{
	if (FDcWriteStateMap* MapState = TryGetTopState<FDcWriteStateMap>(this))
	{
		DC_TRY(MapState->WriteMapEnd());
		PopState<FDcWriteStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteArrayRoot()
{
	FDcBaseWriteState& TopState = GetTopState(this);
	{
		FDcWriteStateArray* ArrayState = TopState.As<FDcWriteStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FDcWriteStateArray::EState::ExpectRoot)
		{
			return ArrayState->WriteArrayRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UArrayProperty::StaticClass(), Datum));

		FDcWriteStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		DC_TRY(ChildArray.WriteArrayRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteArrayEnd()
{
	if (FDcWriteStateArray* ArrayState = TryGetTopState<FDcWriteStateArray>(this))
	{
		DC_TRY(ArrayState->WriteArrayEnd());
		PopState<FDcWriteStateArray>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateArray::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteNil()
{
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteNil();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteReference(UObject* Value)
{
	if (FDcWriteStateClass* ClassState = TryGetTopState<FDcWriteStateClass>(this))
	{
		return ClassState->WriteReference(Value);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FDcWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::SkipWrite()
{
	return GetTopState(this).SkipWrite();
}

FDcResult FDcPropertyWriter::PeekWriteProperty(UField** OutProperty)
{
	return GetTopState(this).PeekWriteProperty(OutProperty);
}

FDcResult FDcPropertyWriter::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	return GetTopState(this).WriteDataEntry(ExpectedPropertyClass, OutDatum);
}

