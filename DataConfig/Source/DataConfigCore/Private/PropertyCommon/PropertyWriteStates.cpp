#include "PropertyWriteStates.h"
#include "DataConfigTypes.h"
#include "PropertyUtils.h"

namespace DataConfig
{

FResult FBaseWriteState::Peek(EDataEntry Next) { return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::WriteName(const FName& Value){ return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum) { return Fail(EErrorCode::UnknownError); }

EPropertyWriteType FWriteStateNil::GetType()
{
	return EPropertyWriteType::Nil;
}

FResult FWriteStateNil::Peek(EDataEntry Next)
{
	return Expect(Next == EDataEntry::Ended, EErrorCode::WriteEndFail);
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

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		TRY((WriteValue<UNameProperty, FName, EErrorCode::WriteNameFail>(*this, Value)));
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

FResult FWriteStateStruct::WriteStructRoot(const FName& Name)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return Expect(Name == StructClass->GetFName(), EErrorCode::WriteStructRootFail);
	}
	else
	{
		return Fail(EErrorCode::WriteStructRootFail);
	}
}

FResult FWriteStateStruct::WriteStructEnd(const FName& Name)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return Expect(Name == StructClass->GetFName(), EErrorCode::WriteStructEndFail);
	}
	else
	{
		return Fail(EErrorCode::WriteStructEndFail);
	}
}

} // namespace DataConfig








