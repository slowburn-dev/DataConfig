#include "DcPropertyWriteStates.h"
#include "DcTypes.h"
#include "DcPropertyUtils.h"

namespace DataConfig
{

FResult FBaseWriteState::Peek(EDataEntry Next) { return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::WriteName(const FName& Value){ return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum) { return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::SkipWrite() { return Fail(EErrorCode::UnknownError); }
FResult FBaseWriteState::GetWriteProperty(UField** OutProperty) { return Fail(EErrorCode::UnknownError); }

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

FResult FWriteStateStruct::SkipWrite()
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::SkipWriteFail);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

FResult FWriteStateStruct::GetWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::GetPropertyFail);

	*OutProperty = Property;
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

EPropertyWriteType FWriteStateClass::GetType()
{
	return EPropertyWriteType::ClassProperty;
}

FResult FWriteStateClass::Peek(EDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return Expect(Next == EDataEntry::ClassRoot, EErrorCode::WriteClassFail);
	}
	else if (State == EState::ExpectNil)
	{
		return Expect(Next == EDataEntry::Nil, EErrorCode::WriteNilFail);
	}
	else if (State == EState::ExpectReference)
	{
		return Expect(Next == EDataEntry::Reference, EErrorCode::WriteReferenceFail);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return Expect(Next == EDataEntry::ClassEnd
			|| Next == EDataEntry::Name,
			EErrorCode::WriteClassKeyFail);
	}
	else if (State == EState::ExpectValue)
	{
		return Expect(Next == PropertyToDataEntry(Datum.Property), EErrorCode::WriteClassValueFail);
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

FResult FWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Datum.Property = NextPropertyByName(Class->PropertyLink, Value);
		if (Datum.Property == nullptr)
			return Fail(EErrorCode::WriteClassKeyFail);

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

FResult FWriteStateClass::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::WriteClassValueFail);

	if (!Datum.Property->IsA(ExpectedPropertyClass))
		return Fail(EErrorCode::WriteClassValueFail);

	UProperty* Property = CastChecked<UProperty>(Datum.Property);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(Datum.DataPtr);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

DataConfig::FResult FWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::WriteClassValueFail);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

FResult FWriteStateClass::GetWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectValue)
		return Fail(EErrorCode::GetPropertyFail);

	*OutProperty = Datum.Property;
	return Ok();
}

DataConfig::FResult FWriteStateClass::WriteClassRoot(const FClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectRoot)
	{
		TRY(Expect(ClassStat.Name == Class->GetFName(), EErrorCode::WriteClassFail));

		if (ClassStat.Reference == EDataReference::NullReference)
		{
			State = EState::ExpectNil;
		}
		else if (ClassStat.Reference == EDataReference::ExternalReference)
		{
			State = EState::ExpectReference;
		}
		else if (ClassStat.Reference == EDataReference::InlineObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
				Datum.DataPtr = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);
				Datum.Property = ObjProperty->PropertyClass;

				if (!Datum.DataPtr)
				{
					//	inline object needs to be created by user
					return Fail(EErrorCode::WriteClassInlineNotCreated);
				}
			}

			State = EState::ExpectKeyOrEnd;
		}
		else
		{
			checkNoEntry();
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteClassFail);
	}
}

FResult FWriteStateClass::WriteClassEnd(const FClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return Expect(ClassStat.Name == Class->GetFName(), EErrorCode::WriteClassEndFail);
	}
	else
	{
		return Fail(EErrorCode::WriteClassEndFail);
	}
}

FResult FWriteStateClass::WriteNil()
{
	if (State == EState::ExpectNil)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, nullptr);

		State = EState::ExpectKeyOrEnd;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteNilFail);
	}
}

FResult FWriteStateClass::WriteReference(UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, Value);

		State = EState::ExpectKeyOrEnd;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteNilFail);
	}
}

EPropertyWriteType FWriteStateMap::GetType()
{
	return EPropertyWriteType::MapProperty;
}

FResult FWriteStateMap::Peek(EDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return Expect(Next == EDataEntry::MapRoot, EErrorCode::WriteMapFail);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return Expect(Next == EDataEntry::MapEnd || Next == PropertyToDataEntry(MapProperty->KeyProp), EErrorCode::WriteMapKeyFail);
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		return Expect(Next == PropertyToDataEntry(MapProperty->ValueProp), EErrorCode::WriteMapValueFail);
	}
	else if (State == EState::Ended)
	{
		return Fail(EErrorCode::WriteMapAfterEnd);
	}
	else
	{
		checkNoEntry();
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FWriteStateMap::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName, EErrorCode::WriteNameFail>(*this, Value);
}

FResult FWriteStateMap::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		//	crucial to construct, future write is copy assignment
		MapHelper.AddDefaultValue_Invalid_NeedsRehash();
		bNeedsRehash = true;
		MapHelper.GetKeyPtr(Index);

		OutDatum.Property = MapHelper.GetKeyProperty();
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);

		OutDatum.Property = MapHelper.GetValueProperty();
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);

		++Index;
		State = EState::ExpectKeyOrEnd;
		return Ok();
	}
	else
	{
		return Fail(FailCode);
	}
}

FResult FWriteStateMap::SkipWrite()
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		++Index;
		State = EState::ExpectKeyOrEnd;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::SkipWriteFail);
	}
}

FResult FWriteStateMap::GetWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		*OutProperty = MapProperty->KeyProp;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = MapProperty->ValueProp;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::GetPropertyFail);
	}
}

FResult FWriteStateMap::WriteMapRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteMapFail);
	}
}

FResult FWriteStateMap::WriteMapEnd()
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;

		if (bNeedsRehash)
		{
			FScriptMapHelper MapHelper(MapProperty, MapPtr);
			MapHelper.Rehash();
		}

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteMapEndFail);
	}
}

EPropertyWriteType FWriteStateArray::GetType()
{
	return EPropertyWriteType::ArrayProperty;
}

FResult FWriteStateArray::Peek(EDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return Expect(Next == EDataEntry::ArrayRoot, EErrorCode::WriteArrayFail);
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		return Expect(Next == EDataEntry::ArrayEnd || Next == PropertyToDataEntry(ArrayProperty->Inner), EErrorCode::WriteArrayFail);
	}
	else if (State == EState::Ended)
	{
		return Fail(EErrorCode::WriteArrayFail);
	}
	else
	{
		checkNoEntry();
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FWriteStateArray::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName, EErrorCode::WriteNameFail>(*this, Value);
}

FResult FWriteStateArray::WriteDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(FailCode);

	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	ArrayHelper.AddValue();
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	++Index;
	return Ok();
}

DataConfig::FResult FWriteStateArray::SkipWrite()
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(EErrorCode::SkipWriteFail);

	++Index;
	return Ok();
}

FResult FWriteStateArray::GetWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(EErrorCode::GetPropertyFail);

	*OutProperty = ArrayProperty->Inner;
	return Ok();
}

FResult FWriteStateArray::WriteArrayRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectItemOrEnd;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteArrayFail);
	}
}

FResult FWriteStateArray::WriteArrayEnd()
{
	if (State == EState::ExpectItemOrEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::WriteArrayEndFail);
	}
}

} // namespace DataConfig









