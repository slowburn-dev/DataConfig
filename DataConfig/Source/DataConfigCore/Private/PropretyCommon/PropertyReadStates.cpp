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

FResult FStateStruct::ReadPastRoot()
{
	if (State == EState::ExpectRoot)
	{
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

FResult FStateStruct::ReadStructRoot(FPropertyReader* Self, FName* OutNamePtr, FContextStorage* CtxPtr)
{
	//	why this makes sense even it's a bit weird, is that
	//	this is state based. on different state the same method actually means different things
	//	and we'll need to handle every state anyway.
	if (State == EState::ExpectRoot)
	{
		TRY(ReadPastRoot());

		if (OutNamePtr)
			*OutNamePtr = StructClass->GetFName();

		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(ReadDataEntry(UStructProperty::StaticClass(), EErrorCode::ReadStructFail, CtxPtr, Datum));

		//	this maps to the double Push state
		FStateStruct& ChildStruct = PushStructPropertyState(Self, Datum.DataPtr, Datum.As<UStructProperty>()->Struct);

		TRY(ChildStruct.ReadPastRoot());
		if (OutNamePtr)
			*OutNamePtr = ChildStruct.StructClass->GetFName();

		return Ok();
	}
	else 
	{
		return Fail(EErrorCode::ReadStructFail);
	}
}

FResult FStateStruct::ReadStructEnd(FPropertyReader* Self, FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		PopState<FStateStruct>(Self);
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

} // namespace DataConfig


