#include "PropertyReadStates.h"
#include "DataConfigTypes.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

FStateNil& PushNilState(FPropertyReader* Reader) 
{
	Reader->States.AddDefaulted();
	return Emplace<FStateNil>(GetTopStorage(Reader));
}

FStateClass& PushClassPropertyState(FPropertyReader* Reader, UObject* InClassObject)
{
	Reader->States.AddDefaulted();
	return Emplace<FStateClass>(GetTopStorage(Reader), InClassObject);
}

FStateStruct& PushStructPropertyState(FPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass)
{
	Reader->States.AddDefaulted();
	return Emplace<FStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass);
}

FStateMap& PushMappingPropertyState(FPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

void PopState(FPropertyReader* Reader)
{
	Reader->States.Pop();
	check(Reader->States.Num() >= 1);
}

EDataEntry FBaseState::Peek()
{
	return EDataEntry::Ended;
}

FResult FBaseState::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FBaseState::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FBaseState::EndReadValue()
{
	return Fail(EErrorCode::UnknownError);
}

EPropertyType FStateNil::GetType()
{
	return EPropertyType::Nil;
}

EPropertyType FStateClass::GetType()
{
	return EPropertyType::ClassProperty;
}

EDataEntry FStateClass::Peek()
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

FResult FStateClass::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
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

FResult FStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
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

FResult FStateClass::EndReadValue()
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

FResult FStateClass::ReadClassRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		check(ClassObject);
		UClass* Cls = ClassObject->GetClass();
		if (OutNamePtr)
			*OutNamePtr = Cls->GetFName();

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
	else
	{
		return Fail(EErrorCode::ReadClassFail);
	}
}

FResult FStateClass::ReadClassEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadClassEndFail);
	}
}

EPropertyType FStateStruct::GetType()
{
	return EPropertyType::StructProperty;
}

EDataEntry FStateStruct::Peek()
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

FResult FStateStruct::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
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

FResult FStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
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
		return Fail(EErrorCode::ReadStructKeyFail);
	}
}

FResult FStateStruct::EndReadValue()
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

DataConfig::FResult FStateStruct::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
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

DataConfig::FResult FStateStruct::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		//	!!! note that this doens't need to EndRead, it's done before entering this state
		//TRY(GetTopState(Self).EndReadValue());
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadStructEndFail);
	}
}

EPropertyType FStateMap::GetType()
{
	return EPropertyType::MapProperty;
}

EDataEntry FStateMap::Peek()
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

FResult FStateMap::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.As<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
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

FResult FStateMap::EndReadValue()
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

DataConfig::FResult FStateMap::ReadMapRoot(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		//	TODO check key/value type validity
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

DataConfig::FResult FStateMap::ReadMapEnd(FContextStorage* CtxPtr)
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

} // namespace DataConfig


