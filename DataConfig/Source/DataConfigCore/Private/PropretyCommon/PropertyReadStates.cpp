#include "PropertyReadStates.h"
#include "DataConfigTypes.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

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
	else if (State == EState::Ended)
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

			//	move next
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
			return Fail(FailCode);
		}
	}
	else
	{
		return Fail(EErrorCode::ReadStructKeyFail);
	}
}

FResult FStateStruct::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
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

FResult FStateStruct::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

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


