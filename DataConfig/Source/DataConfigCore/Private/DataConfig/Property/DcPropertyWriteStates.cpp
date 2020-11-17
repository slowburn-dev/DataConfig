#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

FDcResult FDcBaseWriteState::Peek(EDcDataEntry Next) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteName(const FName& Value){ return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::SkipWrite() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::PeekWriteProperty(UField** OutProperty) { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcBaseWriteState::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	checkNoEntry();
}

EDcPropertyWriteType FDcWriteStateNil::GetType()
{
	return EDcPropertyWriteType::Nil;
}

FDcResult FDcWriteStateNil::Peek(EDcDataEntry Next)
{
	return DcExpect(Next == EDcDataEntry::Ended, [=]{
		return DC_FAIL(DcDReadWrite, AlreadyEnded);
	});
}

void FDcWriteStateNil::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatNil(OutSegments, SegType);
}

EDcPropertyWriteType FDcWriteStateStruct::GetType()
{
	return EDcPropertyWriteType::StructProperty;
}

FDcResult FDcWriteStateStruct::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::StructRoot, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::StructRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::StructEnd || Next == EDcDataEntry::Name, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
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
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded);
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateStruct::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = NextPropertyByName(StructClass->PropertyLink, Value);
		if (Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
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
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcWriteStateStruct::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;

	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->ClassConfigName;

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::SkipWrite()
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::PeekWriteProperty(UField** OutProperty)
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
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcWriteStateStruct::WriteStructRoot(const FName& Name)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DC_FAIL(DcDReadWrite, StructNameMismatch)
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FDcWriteStateStruct::WriteStructEnd(const FName& Name)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DC_FAIL(DcDReadWrite, StructNameMismatch)
				<< StructClass->GetFName() << Name;
		});
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

void FDcWriteStateStruct::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatStruct(OutSegments, SegType, StructName, StructClass, Property);
}

EDcPropertyWriteType FDcWriteStateClass::GetType()
{
	return EDcPropertyWriteType::ClassProperty;
}

FDcResult FDcWriteStateClass::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::ClassRoot, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::ClassRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectNil)
	{
		return DcExpect(Next == EDcDataEntry::Nil, [=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::Nil << (int)Next;
		});
	}
	else if (State == EState::ExpectReference)
	{
		return DcExpect(Next == EDcDataEntry::Reference, [=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::Reference << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::ClassEnd || Next == EDcDataEntry::Name,
			[=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
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
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded);
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Datum.Property = NextPropertyByName(Class->PropertyLink, Value);
		if (Datum.Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
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
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcWriteStateClass::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;

	if (!Datum.Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->ClassConfigName << Datum.Property->GetFName() << Datum.Property->GetClass()->GetFName();

	UProperty* Property = CastChecked<UProperty>(Datum.Property);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(Datum.DataPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::PeekWriteProperty(UField** OutProperty)
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
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcWriteStateClass::WriteClassRoot(const FDcClassPropertyStat& ClassStat)
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
					return DC_FAIL(DcDReadWrite, WriteClassInlineNotCreated)
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FDcWriteStateClass::WriteClassEnd(const FDcClassPropertyStat& ClassStat)
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

FDcResult FDcWriteStateClass::WriteNil()
{
	if (State == EState::ExpectNil)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, nullptr);

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectNil << (int)State;
	}
}

FDcResult FDcWriteStateClass::WriteReference(UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, Value);

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State;
	}
}

void FDcWriteStateClass::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatClass(
		OutSegments,
		SegType, 
		reinterpret_cast<UObject*>(Datum.DataPtr),
		Class,
		Datum.CastChecked<UProperty>()
	);
}

EDcPropertyWriteType FDcWriteStateMap::GetType()
{
	return EDcPropertyWriteType::MapProperty;
}

FDcResult FDcWriteStateMap::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::MapRoot, [=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::MapRoot << (int)Next;
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return DcExpect(Next == EDcDataEntry::MapEnd || Next == PropertyToDataEntry(MapProperty->KeyProp),
			[=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
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
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)Actual << (int)Next;
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded);
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateMap::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateMap::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FDcWriteStateMap::SkipWrite()
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FDcWriteStateMap::PeekWriteProperty(UField** OutProperty)
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FDcWriteStateMap::WriteMapRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FDcWriteStateMap::WriteMapEnd()
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
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State;
	}
}

void FDcWriteStateMap::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatMap(OutSegments, SegType, MapProperty, Index,
		State == EState::ExpectKeyOrEnd || State == EState::ExpectValue);
}

EDcPropertyWriteType FDcWriteStateArray::GetType()
{
	return EDcPropertyWriteType::ArrayProperty;
}

FDcResult FDcWriteStateArray::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::ArrayRoot, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
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
				return DC_FAIL(DcDReadWrite, DataTypeMismatch)
					<< (int)Actual << (int)Next;
			}
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded);
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateArray::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateArray::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State;

	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	ArrayHelper.AddValue();
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	++Index;
	return DcOk();
}

FDcResult FDcWriteStateArray::SkipWrite()
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State;

	++Index;
	return DcOk();
}

FDcResult FDcWriteStateArray::PeekWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State;

	*OutProperty = ArrayProperty->Inner;
	return DcOk();
}

FDcResult FDcWriteStateArray::WriteArrayRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectItemOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FDcWriteStateArray::WriteArrayEnd()
{
	if (State == EState::ExpectItemOrEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State;
	}
}

void FDcWriteStateArray::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatArray(OutSegments, SegType, ArrayProperty, Index,
		State == EState::ExpectItemOrEnd);
}







