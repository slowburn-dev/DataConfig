#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

FDcResult FBaseWriteState::Peek(EDcDataEntry Next) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FBaseWriteState::WriteName(const FName& Value){ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FBaseWriteState::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FBaseWriteState::SkipWrite() { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FBaseWriteState::PeekWriteProperty(UField** OutProperty) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }

EPropertyWriteType FWriteStateNil::GetType()
{
	return EPropertyWriteType::Nil;
}

FDcResult FWriteStateNil::Peek(EDcDataEntry Next)
{
	return DcExpect(Next == EDcDataEntry::Ended, [=]{
		return DcFail(DC_DIAG(DcDReadWrite, AlreadyEnded));
	});
}

EPropertyWriteType FWriteStateStruct::GetType()
{
	return EPropertyWriteType::StructProperty;
}

FDcResult FWriteStateStruct::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::StructRoot, [=]{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::StructRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::StructEnd || Next == EDcDataEntry::Name, [=]{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch2))
				<< (int)EDcDataEntry::StructEnd << (int) EDcDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		EDcDataEntry Actual = PropertyToDataEntry(Property);
		if (Next == Actual)
		{
			return DcOk();
		}
		else
		{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DcFail(DC_DIAG(DcDReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
	}
}

FDcResult FWriteStateStruct::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = NextPropertyByName(StructClass->PropertyLink, Value);
		if (Property == nullptr)
			return DcFail(DC_DIAG(DcDReadWrite, CantFindPropertyByName))
				<< Value;

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		DC_TRY((WriteValue<UNameProperty, FName>(*this, Value)));
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FWriteStateStruct::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	if (!Property->IsA(ExpectedPropertyClass))
		return DcFail(DC_DIAG(DcDReadWrite, PropertyMismatch))
			<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->ClassConfigName;

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FWriteStateStruct::SkipWrite()
{
	if (State != EState::ExpectValue)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FWriteStateStruct::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		*OutProperty = StructClass;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = Property;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FWriteStateStruct::WriteStructRoot(const FName& Name)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DcFail(DC_DIAG(DcDReadWrite, StructNameMismatch))
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FWriteStateStruct::WriteStructEnd(const FName& Name)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DcFail(DC_DIAG(DcDReadWrite, StructNameMismatch))
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

EPropertyWriteType FWriteStateClass::GetType()
{
	return EPropertyWriteType::ClassProperty;
}

FDcResult FWriteStateClass::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::ClassRoot, [=]{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::ClassRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectNil)
	{
		return DcExpect(Next == EDcDataEntry::Nil, [=] {
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::Nil << (int)Next;
		});
	}
	else if (State == EState::ExpectReference)
	{
		return DcExpect(Next == EDcDataEntry::Reference, [=] {
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::Reference << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::ClassEnd || Next == EDcDataEntry::Name,
			[=] {
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch2))
				<< (int)EDcDataEntry::ClassEnd << (int)EDcDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(!Datum.IsNone());
		EDcDataEntry Actual = PropertyToDataEntry(Datum.Property);
		if (Next == Actual)
		{
			return DcOk();
		}
		else
		{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DcFail(DC_DIAG(DcDReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
	}
}

FDcResult FWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Datum.Property = NextPropertyByName(Class->PropertyLink, Value);
		if (Datum.Property == nullptr)
			return DcFail(DC_DIAG(DcDReadWrite, CantFindPropertyByName))
				<< Value.ToString();

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		DC_TRY((WriteValue<UNameProperty, FName>(*this, Value)));
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FWriteStateClass::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	if (!Datum.Property->IsA(ExpectedPropertyClass))
		return DcFail(DC_DIAG(DcDReadWrite, PropertyMismatch))
			<< ExpectedPropertyClass->ClassConfigName << Datum.Property->GetFName() << Datum.Property->GetClass()->GetFName();

	UProperty* Property = CastChecked<UProperty>(Datum.Property);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(Datum.DataPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectValue)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FWriteStateClass::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		*OutProperty = Class;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = Datum.Property;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FWriteStateClass::WriteClassRoot(const FDcClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectRoot)
	{
		//	TODO may pass in derived class already
		//TRY(Expect(ClassStat.Name == Class->GetFName()));
		if (ClassStat.Reference == EDcDataReference::NullReference)
		{
			State = EState::ExpectNil;
		}
		else if (ClassStat.Reference == EDcDataReference::ExternalReference)
		{
			State = EState::ExpectReference;
		}
		else if (ClassStat.Reference == EDcDataReference::ExpandObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
				Datum.DataPtr = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);
				Datum.Property = ObjProperty->PropertyClass;

				if (!Datum.DataPtr)
				{
					//	inline object needs to be created by user
					return DcFail(DC_DIAG(DcDReadWrite, WriteClassInlineNotCreated))
						<< ObjProperty->GetFName() << ObjProperty->GetClass()->GetFName();
				}
			}

			State = EState::ExpectKeyOrEnd;
		}
		else
		{
			checkNoEntry();
		}

		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FWriteStateClass::WriteClassEnd(const FDcClassPropertyStat& ClassStat)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		//	TODO now may pass in derived class
		//return Expect(ClassStat.Name == Class->GetFName());
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

FDcResult FWriteStateClass::WriteNil()
{
	if (State == EState::ExpectNil)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, nullptr);

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectNil << (int)State;
	}
}

FDcResult FWriteStateClass::WriteReference(UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, Value);

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectReference << (int)State;
	}
}

EPropertyWriteType FWriteStateMap::GetType()
{
	return EPropertyWriteType::MapProperty;
}

FDcResult FWriteStateMap::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::MapRoot, [=] {
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::MapRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return DcExpect(Next == EDcDataEntry::MapEnd || Next == PropertyToDataEntry(MapProperty->KeyProp), 
			[=] {
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch2))
				<< (int)EDcDataEntry::ClassEnd << (int)EDcDataEntry::Name << (int)Next;
		});
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		EDcDataEntry Actual = PropertyToDataEntry(MapProperty->ValueProp);
		if (Next == Actual)
		{
			return DcOk();
		}
		else
		{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DcFail(DC_DIAG(DcDReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
	}
}

FDcResult FWriteStateMap::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FDcResult FWriteStateMap::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
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
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);

		OutDatum.Property = MapHelper.GetValueProperty();
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);

		++Index;
		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FWriteStateMap::SkipWrite()
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		++Index;
		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FWriteStateMap::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		*OutProperty = MapProperty->KeyProp;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		*OutProperty = MapProperty->ValueProp;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect2))
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FWriteStateMap::WriteMapRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FWriteStateMap::WriteMapEnd()
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;

		if (bNeedsRehash)
		{
			FScriptMapHelper MapHelper(MapProperty, MapPtr);
			MapHelper.Rehash();
		}

		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

EPropertyWriteType FWriteStateArray::GetType()
{
	return EPropertyWriteType::ArrayProperty;
}

FDcResult FWriteStateArray::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::ArrayRoot, [=]{
			return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
				<< (int)EDcDataEntry::ArrayRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		if (Next == EDcDataEntry::ArrayEnd)
		{
			return DcOk();
		}
		else
		{
			EDcDataEntry Actual = PropertyToDataEntry(ArrayProperty->Inner);
			if (Next == Actual)
			{
				return DcOk();
			}
			else
			{
				return DcFail(DC_DIAG(DcDReadWrite, DataTypeMismatch))
					<< (int)Actual << (int)Next;
			}
		}
	}
	else if (State == EState::Ended)
	{
		return DcFail(DC_DIAG(DcDReadWrite, AlreadyEnded));
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
	}
}

FDcResult FWriteStateArray::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FDcResult FWriteStateArray::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	ArrayHelper.AddValue();
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	++Index;
	return DcOk();
}

FDcResult FWriteStateArray::SkipWrite()
{
	if (State != EState::ExpectItemOrEnd)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

	++Index;
	return DcOk();
}

FDcResult FWriteStateArray::PeekWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;

	*OutProperty = ArrayProperty->Inner;
	return DcOk();
}

FDcResult FWriteStateArray::WriteArrayRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectItemOrEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FWriteStateArray::WriteArrayEnd()
{
	if (State == EState::ExpectItemOrEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItemOrEnd << (int)State;
	}
}








