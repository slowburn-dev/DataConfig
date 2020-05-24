#include "Writer/PropertyWriter.h"

namespace DataConfig {

static FORCEINLINE WriterState& GetTopState(FPropertyWriter* Self)
{
	return *reinterpret_cast<WriterState*>(&Self->States.Top().ImplStorage);
}

template<typename TState>
static TState& GetTopState(FPropertyWriter* Self) {
	return GetTopState(Self).Get<TState>();
}

template<typename TState>
static TState* TryGetTopState(FPropertyWriter* Self) {
	return GetTopState(Self).TryGet<TState>();
}

static void PushNilState(FPropertyWriter* Writer) {
	Writer->States.AddDefaulted();
}

static void PushClassRootState(FPropertyWriter* Writer, UObject* ClassObject)
{
	Writer->States.AddDefaulted();
	GetTopState(Writer).Emplace<StateClassRoot>(ClassObject);
}

static void PushStructRootState(FPropertyWriter* Writer, void* StructPtr, UScriptStruct* StructClass)
{
	Writer->States.AddDefaulted();
	GetTopState(Writer).Emplace<StateStructRoot>(StructPtr, StructClass);
}

static void PushClassPropertyState(FPropertyWriter* Writer, UObject* ClassObject, UProperty* Property)
{
	Writer->States.AddDefaulted();
	GetTopState(Writer).Emplace<StateClassProperty>(ClassObject, Property);
}

static void PushStructPropertyState(FPropertyWriter* Writer, void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
{
	Writer->States.AddDefaulted();
	GetTopState(Writer).Emplace<StateStructProperty>(StructPtr, StructClass, Property);
}

static void PopState(FPropertyWriter* Writer)
{
	Writer->States.Pop();
	check(Writer->States.Num() >= 1);
}

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode>
FResult TryWritePrimitive(FPropertyWriter* Writer, const TPrimitive& Value)
{
	if (GetTopState(Writer).IsType<StateNil>())
	{
		return Fail(EErrorCode::WriteAfterEnded);
	}
	else if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(Writer))
	{
		if (TProperty* WriteProperty = Cast<TProperty>(ClassPropertyState->Property))
		{
			WriteProperty->SetPropertyValue(ClassPropertyState->ClassObject, Value);
			return Ok();
		}
		else
		{
			return Fail(ErrCode);
		}
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(Writer))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			//	should be already handled in `WriteName` 
			return Fail(EErrorCode::WriteStructKeyFail);
		}
		else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			if (TProperty* WriteProperty = Cast<TProperty>(StructPropertyState->Property))
			{
				WriteProperty->SetPropertyValue(
					WriteProperty->ContainerPtrToValuePtr<TPrimitive>(StructPropertyState->StructPtr),
					Value
				);
				//	note that this is now expect key OR expect end
				StructPropertyState->State = StateStructProperty::EState::ExpectKey;
				return Ok();
			}
			else
			{
				return Fail(ErrCode);
			}
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return Fail(EErrorCode::WriteStructEndFail);
		}
		checkNoEntry();
	}
	else if (StatePrimitive* PrimitiveState = TryGetTopState<StatePrimitive>(Writer))
	{
		if (TProperty* WriteProperty = Cast<TProperty>(PrimitiveState->Property))
		{
			WriteProperty->SetPropertyValue(PrimitiveState->PrimitivePtr, Value);
		}
		else
		{
			return Fail(ErrCode);
		}
	}

	return Fail(EErrorCode::UnknownError);
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
	else if (Datum.Property->IsA<UClassProperty>())
	{
		UObject* Obj = reinterpret_cast<UObject*>(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassRootState(this, Obj);
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructRootState(this,
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
	if (StateNil* NilState = TryGetTopState<StateNil>(this))
	{
		return FailIf(Next != EDataEntry::Ended, EErrorCode::WriteEndFail);
	}
	else if (StatePrimitive* PrimtiveState = TryGetTopState<StatePrimitive>(this))
	{
		return FailIf(PropertyToDataEntry(PrimtiveState->Property) != Next, GetWriteErrorCode(Next));
	}
	else if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(this))
	{
		return FailIf(PropertyToDataEntry(ClassPropertyState->Property) != Next, GetWriteErrorCode(Next));
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			//	OK to end in waiting key
			if (Next == EDataEntry::StructEnd)
				return Ok();
			return FailIf(EDataEntry::Name != Next, EErrorCode::WriteStructKeyFail);
		}
		else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			return FailIf(PropertyToDataEntry(StructPropertyState->Property) != Next, GetWriteErrorCode(Next));
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return FailIf(EDataEntry::StructEnd != Next, EErrorCode::WriteStructEndFail);
		}
	}
	else if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		return FailIf(EDataEntry::StructRoot != Next, EErrorCode::WriteStructRootFail);
	}

	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyWriter::WriteBool(bool Value)
{
	return TryWritePrimitive<bool, UBoolProperty, EErrorCode::WriteBoolFail>(this, Value);
}

FResult FPropertyWriter::WriteName(const FName& Value)
{
	if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(this))
	{
		//	TODO
		return TryWritePrimitive<FName, UNameProperty, EErrorCode::WriteNameFail>(this, Value);
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			//	actually not writing anything, just selecting property to write
			//	TODO always starting from first for now
			UProperty* TargetProperty = NextPropertyByName(StructPropertyState->StructClass->PropertyLink, Value);
			if (TargetProperty == nullptr)
			{
				return Fail(EErrorCode::WriteStructKeyFail);
			}
			else
			{
				StructPropertyState->Property = TargetProperty;
				StructPropertyState->State = StateStructProperty::EState::ExpectValue;
			}
			return Ok();
		}
	}

	return TryWritePrimitive<FName, UNameProperty, EErrorCode::WriteNameFail>(this, Value);
}

FResult FPropertyWriter::WriteString(const FString& Value)
{
	return TryWritePrimitive<FString, UStrProperty, EErrorCode::WriteStringFail>(this, Value);
}

FResult FPropertyWriter::WriteStructRoot(const FName& Name)
{
	if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		//	TODO check the Name against struct name
		PushStructPropertyState(this,
			StructRootState->StructPtr,
			StructRootState->StructClass,
			nullptr	// write null as the key is determined at next write
		);
		return Ok();
	}

	return Fail(EErrorCode::WriteStructRootFail);
}

FResult FPropertyWriter::WriteStructEnd(const FName& Name)
{
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			//	TODO check the Name against struct name
			PopState(this);
			PopState(this);
			//	TODO finish top state value write
			return Ok();
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended
			|| StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			return Fail(EErrorCode::WriteStructEndFail);
		}
	}


	return Fail(EErrorCode::WriteStructEndFail);
}

FPropertyWriter::FPropertyState::FPropertyState()
{
	static_assert(sizeof(FPropertyState) <= 64, "larger than cacheline");
	static_assert(sizeof(WriterState) <= sizeof(FPropertyState), "impl storage size too small");
	new(&ImplStorage) ReaderState(TInPlaceType<StateNil>{});
}

} // namespace DataConfig
