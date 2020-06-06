#include "PropertyWriteStates.h"
#include "DataConfigTypes.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig
{

EPropertyWriteType FWriteStateNil::GetType()
{
	return EPropertyWriteType::Nil;
}

EPropertyWriteType FWriteStateStruct::GetType()
{
	return EPropertyWriteType::StructProperty;
}

FResult FWriteStateStruct::Peek(EDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return Expect(Next == EDataEntry::StructRoot, EErrorCode::WriteStructRootFail);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return Expect(Next == EDataEntry::StructEnd || Next == EDataEntry::Name, EErrorCode::WriteStructKeyFail);
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return Expect(Next == PropertyToDataEntry(Property), EErrorCode::WriteStructRootFail);
	}
	else if (State == EState::Ended)
	{
		return Fail(EErrorCode::WriteStructAfterEnd);
	}
	else
	{
		checkNoEntry();
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FWriteStateStruct::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = NextPropertyByName(StructClass->PropertyLink, Value);
		if (Property == nullptr)
			return Fail(EErrorCode::WriteStructKeyFail);

		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(WriteDataEntry(UNameProperty::StaticClass(), EErrorCode::WriteNameFail, Datum));

		Datum.As<UNameProperty>()->SetPropertyValue(Datum.DataPtr, Value);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteNameFail);
	}
}

FResult FWriteStateStruct::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::WriteStructValueFail);

	if (!Property->IsA(ExpectedPropertyClass))
		return Fail(EErrorCode::WriteStructValueFail);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

} // namespace DataConfig








