#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyWriteStates.h"

static FORCEINLINE FBaseWriteState& GetTopState(FDcPropertyWriter* Self)
{
	return *reinterpret_cast<FBaseWriteState*>(&Self->States.Top().ImplStorage);
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

static FWriteStateNil& PushNilState(FDcPropertyWriter* Writer) {
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateNil>(GetTopStorage(Writer));
}

static FWriteStateClass& PushClassRootState(FDcPropertyWriter* Writer, UObject* InClassObject, UClass* InClass)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateClass>(GetTopStorage(Writer), InClassObject, InClass);
}

static FWriteStateClass& PushClassPropertyState(FDcPropertyWriter* Writer, void* InDataPtr, UObjectProperty* InObjProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateClass>(GetTopStorage(Writer), InDataPtr, InObjProperty);
}

static FWriteStateStruct& PushStructPropertyState(FDcPropertyWriter* Writer, void* InStructPtr, UScriptStruct* InStructStruct)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateStruct>(GetTopStorage(Writer), InStructPtr, InStructStruct);
}

static FWriteStateMap& PushMappingPropertyState(FDcPropertyWriter* Writer, void* InMapPtr, UMapProperty* InMapProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateMap>(GetTopStorage(Writer), InMapPtr, InMapProperty);
}

static FWriteStateArray& PushArrayPropertyState(FDcPropertyWriter* Writer, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateArray>(GetTopStorage(Writer), InArrayPtr, InArrayProperty);
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
	FBaseWriteState& TopState = GetTopState(this);
	{
		FWriteStateStruct* StructState = TopState.As<FWriteStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FWriteStateStruct::EState::ExpectRoot)
		{
			return StructState->WriteStructRoot(Name);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UStructProperty::StaticClass(), Datum));

		FWriteStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.CastChecked<UStructProperty>()->Struct);
		DC_TRY(ChildStruct.WriteStructRoot(Name));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteStructEnd(const FName& Name)
{
	if (FWriteStateStruct* StructState = TryGetTopState<FWriteStateStruct>(this))
	{
		DC_TRY(StructState->WriteStructEnd(Name));
		PopState<FWriteStateStruct>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateStruct::ID << (int)GetTopState(this).GetType();
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
	FBaseWriteState& TopState = GetTopState(this);
	{
		FWriteStateClass* ClassState = TopState.As<FWriteStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FWriteStateClass::EState::ExpectRoot)
		{
			return ClassState->WriteClassRoot(Class);
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UObjectProperty::StaticClass(), Datum));

		PushTopClassPropertyState(Datum);
		DC_TRY(GetTopState(this).As<FWriteStateClass>()->WriteClassRoot(Class));
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteClassEnd(const FDcClassPropertyStat& Class)
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		DC_TRY(ClassState->WriteClassEnd(Class));
		PopState<FWriteStateClass>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteMapRoot()
{
	FBaseWriteState& TopState = GetTopState(this);
	{
		FWriteStateMap* MapState = TopState.As<FWriteStateMap>();
		if (MapState != nullptr
			&& MapState->State == FWriteStateMap::EState::ExpectRoot)
		{
			return MapState->WriteMapRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UMapProperty::StaticClass(), Datum));

		FWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		DC_TRY(ChildMap.WriteMapRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteMapEnd()
{
	if (FWriteStateMap* MapState = TryGetTopState<FWriteStateMap>(this))
	{
		DC_TRY(MapState->WriteMapEnd());
		PopState<FWriteStateMap>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteArrayRoot()
{
	FBaseWriteState& TopState = GetTopState(this);
	{
		FWriteStateArray* ArrayState = TopState.As<FWriteStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FWriteStateArray::EState::ExpectRoot)
		{
			return ArrayState->WriteArrayRoot();
		}
	}

	{
		FDcPropertyDatum Datum;
		DC_TRY(TopState.WriteDataEntry(UArrayProperty::StaticClass(), Datum));

		FWriteStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		DC_TRY(ChildArray.WriteArrayRoot());
	}

	return DcOk();
}

FDcResult FDcPropertyWriter::WriteArrayEnd()
{
	if (FWriteStateArray* ArrayState = TryGetTopState<FWriteStateArray>(this))
	{
		DC_TRY(ArrayState->WriteArrayEnd());
		PopState<FWriteStateArray>(this);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateArray::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteNil()
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		return ClassState->WriteNil();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FDcResult FDcPropertyWriter::WriteReference(UObject* Value)
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		return ClassState->WriteReference(Value);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
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

