#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

namespace DataConfig
{

FResult FBaseWriteState::Peek(EDataEntry Next) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FBaseWriteState::WriteName(const FName& Value){ return Fail(DIAG(DCommon, NotImplemented)); }
FResult FBaseWriteState::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FBaseWriteState::SkipWrite() { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FBaseWriteState::PeekWriteProperty(UField** OutProperty) { return Fail(DIAG(DCommon, NotImplemented)); }

EPropertyWriteType FWriteStateNil::GetType()
{
	return EPropertyWriteType::Nil;
}

FResult FWriteStateNil::Peek(EDataEntry Next)
{
	return Expect(Next == EDataEntry::Ended, [=]{
		return Fail(DIAG(DReadWrite, AlreadyEnded));
	});
}

EPropertyWriteType FWriteStateStruct::GetType()
{
	return EPropertyWriteType::StructProperty;
}

FResult FWriteStateStruct::Peek(EDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return Expect(Next == EDataEntry::StructRoot, [=]{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::StructRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return Expect(Next == EDataEntry::StructEnd || Next == EDataEntry::Name, [=]{
			return Fail(DIAG(DReadWrite, DataTypeMismatch2))
				<< (int)EDataEntry::StructEnd << (int) EDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		EDataEntry Actual = PropertyToDataEntry(Property);
		if (Next == Actual)
		{
			return Ok();
		}
		else
		{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return Fail(DIAG(DReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return Fail(DIAG(DCommon, Unreachable));
	}
}

FResult FWriteStateStruct::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = NextPropertyByName(StructClass->PropertyLink, Value);
		if (Property == nullptr)
			return Fail(DIAG(DReadWrite, CantFindPropertyByName))
				<< Value;

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		TRY((WriteValue<UNameProperty, FName>(*this, Value)));
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FResult FWriteStateStruct::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	if (!Property->IsA(ExpectedPropertyClass))
		return Fail(DIAG(DReadWrite, PropertyMismatch))
			<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->ClassConfigName;

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

FResult FWriteStateStruct::SkipWrite()
{
	if (State != EState::ExpectValue)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

FResult FWriteStateStruct::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		*OutProperty = StructClass;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = Property;
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FResult FWriteStateStruct::WriteStructRoot(const FName& Name)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return Expect(Name == StructClass->GetFName(), [=] {
			return Fail(DIAG(DReadWrite, StructNameMismatch))
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FWriteStateStruct::WriteStructEnd(const FName& Name)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return Expect(Name == StructClass->GetFName(), [=] {
			return Fail(DIAG(DReadWrite, StructNameMismatch))
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
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
		return Expect(Next == EDataEntry::ClassRoot, [=]{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::ClassRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectNil)
	{
		return Expect(Next == EDataEntry::Nil, [=] {
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::Nil << (int)Next;
		});
	}
	else if (State == EState::ExpectReference)
	{
		return Expect(Next == EDataEntry::Reference, [=] {
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::Reference << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return Expect(Next == EDataEntry::ClassEnd || Next == EDataEntry::Name,
			[=] {
			return Fail(DIAG(DReadWrite, DataTypeMismatch2))
				<< (int)EDataEntry::ClassEnd << (int)EDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(!Datum.IsNone());
		EDataEntry Actual = PropertyToDataEntry(Datum.Property);
		if (Next == Actual)
		{
			return Ok();
		}
		else
		{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return Fail(DIAG(DReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return Fail(DIAG(DCommon, Unreachable));
	}
}

FResult FWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Datum.Property = NextPropertyByName(Class->PropertyLink, Value);
		if (Datum.Property == nullptr)
			return Fail(DIAG(DReadWrite, CantFindPropertyByName))
				<< Value.ToString();

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		TRY((WriteValue<UNameProperty, FName>(*this, Value)));
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FResult FWriteStateClass::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	if (!Datum.Property->IsA(ExpectedPropertyClass))
		return Fail(DIAG(DReadWrite, PropertyMismatch))
			<< ExpectedPropertyClass->ClassConfigName << Datum.Property->GetFName() << Datum.Property->GetClass()->GetFName();

	UProperty* Property = CastChecked<UProperty>(Datum.Property);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(Datum.DataPtr);

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

DataConfig::FResult FWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectValue)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return Ok();
}

FResult FWriteStateClass::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		*OutProperty = Class;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = Datum.Property;
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FResult FWriteStateClass::WriteClassRoot(const FClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectRoot)
	{
		//	TODO may pass in derived class already
		//TRY(Expect(ClassStat.Name == Class->GetFName()));
		if (ClassStat.Reference == EDataReference::NullReference)
		{
			State = EState::ExpectNil;
		}
		else if (ClassStat.Reference == EDataReference::ExternalReference)
		{
			State = EState::ExpectReference;
		}
		else if (ClassStat.Reference == EDataReference::ExpandObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
				Datum.DataPtr = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);
				Datum.Property = ObjProperty->PropertyClass;

				if (!Datum.DataPtr)
				{
					//	inline object needs to be created by user
					return Fail(DIAG(DReadWrite, WriteClassInlineNotCreated))
						<< ObjProperty->GetFName() << ObjProperty->GetClass()->GetFName();
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FWriteStateClass::WriteClassEnd(const FClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		//	TODO now may pass in derived class
		//return Expect(ClassStat.Name == Class->GetFName());
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectNil << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectReference << (int)State;
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
		return Expect(Next == EDataEntry::MapRoot, [=] {
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::MapRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return Expect(Next == EDataEntry::MapEnd || Next == PropertyToDataEntry(MapProperty->KeyProp), 
			[=] {
			return Fail(DIAG(DReadWrite, DataTypeMismatch2))
				<< (int)EDataEntry::ClassEnd << (int)EDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		EDataEntry Actual = PropertyToDataEntry(MapProperty->ValueProp);
		if (Next == Actual)
		{
			return Ok();
		}
		else
		{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return Fail(DIAG(DReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return Fail(DIAG(DCommon, Unreachable));
	}
}

FResult FWriteStateMap::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FResult FWriteStateMap::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum)
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FResult FWriteStateMap::PeekWriteProperty(UField** OutProperty)
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
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
		return Expect(Next == EDataEntry::ArrayRoot, [=]{
			return Fail(DIAG(DReadWrite, DataTypeMismatch))
				<< (int)EDataEntry::ArrayRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		if (Next == EDataEntry::ArrayEnd)
		{
			return Ok();
		}
		else
		{
			EDataEntry Actual = PropertyToDataEntry(ArrayProperty->Inner);
			if (Next == Actual)
			{
				return Ok();
			}
			else
			{
				return Fail(DIAG(DReadWrite, DataTypeMismatch))
					<< (int)Actual << (int)Next;
			}
		}
	}
	else if (State == EState::Ended)
	{
		return Fail(DIAG(DReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return Fail(DIAG(DCommon, Unreachable));
	}
}

FResult FWriteStateArray::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FResult FWriteStateArray::WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	ArrayHelper.AddValue();
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	++Index;
	return Ok();
}

FResult FWriteStateArray::SkipWrite()
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

	++Index;
	return Ok();
}

FResult FWriteStateArray::PeekWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
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
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;
	}
}

} // namespace DataConfig









