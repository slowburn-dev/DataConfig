#include "DcPropertyReadStates.h"
#include "DcTypes.h"
#include "DcPropertyUtils.h"

namespace DataConfig {

EDataEntry FBaseReadState::Peek()
{
	return EDataEntry::Ended;
}

FResult FBaseReadState::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FBaseReadState::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	return Fail(EErrorCode::UnknownError);
}

EPropertyReadType FReadStateNil::GetType()
{
	return EPropertyReadType::Nil;
}

EPropertyReadType FReadStateClass::GetType()
{
	return EPropertyReadType::ClassProperty;
}

EDataEntry FReadStateClass::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::ClassRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::ClassEnd;
	}
	else if (State == EState::ExpectKey)
	{
		return EDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateClass::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.As<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadNameFail);
	}
}

FResult FReadStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// should've bee already handled in `ReadName`
		return Fail(EErrorCode::ReadClassKeyFail);
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);
		if (Property->IsA(ExpectedPropertyClass))
		{
			OutDatum.Property = Property;
			OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(ClassObject);

			EndReadValue();
			return Ok();
		}
		else
		{
			return Fail(FailCode);
		}
	}
	else
	{
		return Fail(EErrorCode::ReadClassKeyFail);
	}
}

FResult FReadStateClass::EndReadValue()
{
	if (State == EState::ExpectValue)
	{
		Property = NextEffectiveProperty(Property);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadClassNextFail);
	}
}

DataConfig::FResult FReadStateClass::ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassObject == nullptr)
		{
			if (OutClassPtr)
			{
				OutClassPtr->Name = Class->GetFName();
				OutClassPtr->Reference = EDataReference::NullReference;
			}

			State = EState::ExpectNull;
			return Ok();
		}
		else
		{
			check(ClassObject);
			UClass* Cls = ClassObject->GetClass();
			if (OutClassPtr)
			{
				OutClassPtr->Name = Cls->GetFName();
				//	TODO handle reference later
				OutClassPtr->Reference = EDataReference::InlineObject;
			}

			Property = FirstEffectiveProperty(Cls->PropertyLink);
			if (Property == nullptr)
			{
				State = EState::ExpectEnd;
			}
			else
			{
				State = EState::ExpectKey;
			}

			return Ok();
		}
	}
	else
	{
		return Fail(EErrorCode::ReadClassFail);
	}
}

DataConfig::FResult FReadStateClass::ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		check(ClassObject);

		if (OutClassPtr)
		{
			OutClassPtr->Name = ClassObject->GetClass()->GetFName();
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadClassEndFail);
	}
}

EPropertyReadType FReadStateStruct::GetType()
{
	return EPropertyReadType::StructProperty;
}

EDataEntry FReadStateStruct::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::StructRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::StructEnd;
	}
	else if (State == EState::ExpectKey)
	{
		return EDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else if (State == EState::Ended)
	{
		return EDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateStruct::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.As<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadStructKeyFail);
	}
}

FResult FReadStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// shouldn't be hanndled in `ReadName`
		return Fail(EErrorCode::ReadStructKeyFail);
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);
		if (Property->IsA(ExpectedPropertyClass))
		{
			OutDatum.Property = Property;
			OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

			EndReadValue();	// this now guarentee success
			return Ok();
		}
		else
		{
			return Fail(FailCode);
		}
	}
	else
	{
		return Fail(EErrorCode::ReadStructKeyFail);
	}
}

FResult FReadStateStruct::EndReadValue()
{
	if (State == EState::ExpectValue)
	{
		Property = NextEffectiveProperty(Property);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadStructNextFail);
	}
}

FResult FReadStateStruct::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (OutNamePtr)
			*OutNamePtr = StructClass->GetFName();

		Property = FirstEffectiveProperty(StructClass->PropertyLink);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadStructFail);
	}
}

FResult FReadStateStruct::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutNamePtr)
		{
			*OutNamePtr = StructClass->GetFName();
		}

		//	!!! note that this doens't need to EndRead, it's done before entering this state
		//TRY(GetTopState(Self).EndReadValue());
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadStructEndFail);
	}
}

EPropertyReadType FReadStateMap::GetType()
{
	return EPropertyReadType::MapProperty;
}

EDataEntry FReadStateMap::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::MapRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::MapEnd;
	}
	else if (State == EState::ExpectKey)
	{
		check(MapProperty);
		return PropertyToDataEntry(MapProperty->KeyProp);
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		return PropertyToDataEntry(MapProperty->ValueProp);
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateMap::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.As<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FReadStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return Fail(FailCode);

	FScriptMapHelper MapHelper(MapProperty, MapPtr);
	if (State == EState::ExpectKey)
	{
		OutDatum.Property = MapHelper.GetKeyProperty();
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);
	}
	else if (State == EState::ExpectValue)
	{
		OutDatum.Property = MapHelper.GetValueProperty();
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);
	}
	else
	{
		checkNoEntry();
	}

	TRY(EndReadValue());
	return Ok();
}

FResult FReadStateMap::EndReadValue()
{
	if (State == EState::ExpectKey)
	{
		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMap* ScriptMap = (FScriptMap*)MapPtr;
		Index += 1;
		if (Index < ScriptMap->Num())
		{
			State = EState::ExpectKey;
		}
		else
		{
			State = EState::ExpectEnd;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FReadStateMap::ReadMapRoot(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		//	check map effectiveness
		if (!IsEffectiveProperty(MapProperty->KeyProp)
			|| !IsEffectiveProperty(MapProperty->ValueProp))
		{
			State = EState::ExpectEnd;
			return Ok();
		}

		FScriptMap* ScriptMap = (FScriptMap*)MapPtr;
		if (ScriptMap->Num() == 0)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadMapFail);
	}
}

FResult FReadStateMap::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadMapEndFail);
	}
}

EPropertyReadType FReadStateArray::GetType()
{
	return EPropertyReadType::ArrayProperty;
}

EDataEntry FReadStateArray::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::ArrayRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::ArrayEnd;
	}
	else if (State == EState::ExpectItem)
	{
		check(ArrayProperty->Inner);
		return PropertyToDataEntry(ArrayProperty->Inner);
	}
	else if (State == EState::Ended)
	{
		return EDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateArray::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.As<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FReadStateArray::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return Fail(FailCode);

	check(State == EState::ExpectItem);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	TRY(EndReadValue());
	return Ok();
}

FResult FReadStateArray::EndReadValue()
{
	if (State == EState::ExpectItem)
	{
		FScriptArray* ScriptArray = (FScriptArray*)ArrayPtr;
		Index += 1;
		if (Index < ScriptArray->Num())
		{
			State = EState::ExpectItem;
		}
		else
		{
			State = EState::ExpectEnd;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FReadStateArray::ReadArrayRoot(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (!IsEffectiveProperty(ArrayProperty->Inner))
		{
			State = EState::ExpectEnd;
			return Ok();
		}

		FScriptArray* ScriptArray = (FScriptArray*)ArrayPtr;
		if (ScriptArray->Num() == 0)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectItem;
		}
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadArrayFail);
	}
}

FResult FReadStateArray::ReadArrayEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadArrayEndFail);
	}
}

} // namespace DataConfig

