#include "Writer/PropertyWriter.h"
#include "PropertyCommon/PropertyWriteStates.h"

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

static FWriteStateClass& PushClassPropertyState(FPropertyWriter* Writer, UObject* InClassObject)
{
	Writer->States.AddDefaulted();
	return Emplace<FWriteStateClass>(GetTopStorage(Writer), InClassObject);
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
		PushClassPropertyState(this, Obj);
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
	return WriteValue<UBoolProperty, bool, EErrorCode::WriteBoolFail>(GetTopState(this), Value);
}

FResult FPropertyWriter::WriteName(const FName& Value)
{
	return GetTopState(this).WriteName(Value);
}

FResult FPropertyWriter::WriteString(const FString& Value)
{
	return WriteValue<UStrProperty, FString, EErrorCode::WriteStringFail>(GetTopState(this), Value);
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
		TRY(TopState.WriteDataEntry(UStructProperty::StaticClass(), EErrorCode::WriteStructRootFail, Datum));
			
		FWriteStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.As<UStructProperty>()->Struct);
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
		return Fail(EErrorCode::WriteStructEndFail);
	}
}

FResult FPropertyWriter::WriteClassRoot(const FName& Name)
{
	FBaseWriteState& TopState = GetTopState(this);
	{
		FWriteStateClass* ClassState = TopState.As<FWriteStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FWriteStateClass::EState::ExpectRoot)
		{
			return ClassState->WriteClassRoot(Name);
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.WriteDataEntry(UClassProperty::StaticClass(), EErrorCode::WriteClassFail, Datum));

		FWriteStateClass& ChildClass = PushClassPropertyState(this, (UObject*)Datum.DataPtr);
		TRY(ChildClass.WriteClassRoot(Name));
	}

	return Ok();
}

FResult FPropertyWriter::WriteClassEnd(const FName& Name)
{
	if (FWriteStateClass* ClassState = TryGetTopState<FWriteStateClass>(this))
	{
		TRY(ClassState->WriteClassEnd(Name));
		PopState<FWriteStateClass>(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteClassEndFail);
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
		TRY(TopState.WriteDataEntry(UMapProperty::StaticClass(), EErrorCode::WriteMapFail, Datum));

		FWriteStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.As<UMapProperty>());
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
		return Fail(EErrorCode::WriteMapEndFail);
	}
}

} // namespace DataConfig
