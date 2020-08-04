#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyWriteStates.h"

namespace DataConfig {

static FORCEINLINE FBaseWriteState& GetTopState(FPropertyWriter* Self)
{
	return *reinterpret_cast<FBaseWriteState*>(&Self->States.Top().ImplStorage);
}

using WriterStorageType = FPropertyWriter::FPropertyState::ImplStorageType;
static FORCEINLINE WriterStorageType* GetTopStorage(FPropertyWriter* Self)
{
	return &Self->States.Top().ImplStorage;
}

template<typename TState>
static TState& GetTopState(FPropertyWriter* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FPropertyWriter* Self) {
	return GetTopState(Self).As<TState>();
}

static FWriteStateNil& PushNilState(FPropertyWriter* Writer) {
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateNil>(GetTopStorage(Writer));
}

static FWriteStateClass& PushClassRootState(FPropertyWriter* Writer, UObject* InClassObject, UClass* InClass)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateClass>(GetTopStorage(Writer), InClassObject, InClass);
}

static FWriteStateClass& PushClassPropertyState(FPropertyWriter* Writer, void* InDataPtr, UObjectProperty* InObjProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateClass>(GetTopStorage(Writer), InDataPtr, InObjProperty);
}

static FWriteStateStruct& PushStructPropertyState(FPropertyWriter* Writer, void* InStructPtr, UScriptStruct* InStructStruct)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateStruct>(GetTopStorage(Writer), InStructPtr, InStructStruct);
}

static FWriteStateMap& PushMappingPropertyState(FPropertyWriter* Writer, void* InMapPtr, UMapProperty* InMapProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateMap>(GetTopStorage(Writer), InMapPtr, InMapProperty);
}

static FWriteStateArray& PushArrayPropertyState(FPropertyWriter* Writer, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateArray>(GetTopStorage(Writer), InArrayPtr, InArrayProperty);
}

static void PopState(FPropertyWriter* Writer)
{
	Writer->States.Pop();
	check(Writer->States.Num() >= 1);
}

template<typename TState>
static void PopState(FPropertyWriter* Writer)
{
	check(TState::ID == GetTopState(Writer).GetType());
	PopState(Writer);
}

FPropertyWriter::FPropertyWriter()
{
	PushNilState(this);
}

FPropertyWriter::FPropertyWriter(FPropertyDatum Datum)
	: FPropertyWriter()
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

FResult FPropertyWriter::Peek(EDataEntry Next)
{
	return GetTopState(this).Peek(Next);
}

FResult FPropertyWriter::WriteBool(bool Value)
{
	return WriteValue<UBoolProperty, bool>(GetTopState(this), Value);
}

FResult FPropertyWriter::WriteName(const FName& Value)
{
	return GetTopState(this).WriteName(Value);
}

FResult FPropertyWriter::WriteString(const FString& Value)
{
	return WriteValue<UStrProperty, FString>(GetTopState(this), Value);
}

FResult FPropertyWriter::WriteStructRoot(const FName& Name)
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
		FPropertyDatum Datum;
		TRY(TopState.WriteDataEntry(UStructProperty::StaticClass(), Datum));

		FWriteStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.CastChecked<UStructProperty>()->Struct);
		TRY(ChildStruct.WriteStructRoot(Name));
	}

	return Ok();
}

FResult FPropertyWriter::WriteStructEnd(const FName& Name)
{
	if (FWriteStateStruct* StructState = TryGetTopState<FWriteStateStruct>(this))
	{
		TRY(StructState->WriteStructEnd(Name));
		PopState<FWriteStateStruct>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

void FPropertyWriter::PushTopClassPropertyState(FPropertyDatum& Datum)
{
	UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
	check(ObjProperty);
	PushClassPropertyState(this, Datum.DataPtr, ObjProperty);
}


FResult FPropertyWriter::WriteClassRoot(const FClassPropertyStat& Class)
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
		FPropertyDatum Datum;
		TRY(TopState.WriteDataEntry(UObjectProperty::StaticClass(), Datum));

		PushTopClassPropertyState(Datum);
		TRY(GetTopState(this).As<FWriteStateClass>()->WriteClassRoot(Class));
	}

	return Ok();
}

FResult FPropertyWriter::WriteClassEnd(const FClassPropertyStat& Class)
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		TRY(ClassState->WriteClassEnd(Class));
		PopState<FWriteStateClass>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyWriter::WriteMapRoot()
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
		FPropertyDatum Datum;
		TRY(TopState.WriteDataEntry(UMapProperty::StaticClass(), Datum));

		FWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		TRY(ChildMap.WriteMapRoot());
	}

	return Ok();
}

FResult FPropertyWriter::WriteMapEnd()
{
	if (FWriteStateMap* MapState = TryGetTopState<FWriteStateMap>(this))
	{
		TRY(MapState->WriteMapEnd());
		PopState<FWriteStateMap>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyWriter::WriteArrayRoot()
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
		FPropertyDatum Datum;
		TRY(TopState.WriteDataEntry(UArrayProperty::StaticClass(), Datum));

		FWriteStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		TRY(ChildArray.WriteArrayRoot());
	}

	return Ok();
}

FResult FPropertyWriter::WriteArrayEnd()
{
	if (FWriteStateArray* ArrayState = TryGetTopState<FWriteStateArray>(this))
	{
		TRY(ArrayState->WriteArrayEnd());
		PopState<FWriteStateArray>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateArray::ID << (int)GetTopState(this).GetType();
	}
}

DataConfig::FResult FPropertyWriter::WriteNil()
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		return ClassState->WriteNil();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

DataConfig::FResult FPropertyWriter::WriteReference(UObject* Value)
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		return ClassState->WriteReference(Value);
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FWriteStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyWriter::SkipWrite()
{
	return GetTopState(this).SkipWrite();
}

FResult FPropertyWriter::PeekWriteProperty(UField** OutProperty)
{
	return GetTopState(this).PeekWriteProperty(OutProperty);
}

DataConfig::FResult FPropertyWriter::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum)
{
	return GetTopState(this).WriteDataEntry(ExpectedPropertyClass, OutDatum);
}

} // namespace DataConfig
