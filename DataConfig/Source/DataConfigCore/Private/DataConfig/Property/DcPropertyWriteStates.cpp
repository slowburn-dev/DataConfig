#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"

FDcPropertyWriter* _GetStackWriter()
{
	return (FDcPropertyWriter*)(DcEnv().WriterStack.Top());
}

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
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
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
				<< (int)EDcDataEntry::StructRoot << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::StructEnd || Next == EDcDataEntry::Name, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
				<< (int)EDcDataEntry::StructEnd << (int) EDcDataEntry::Name << (int)Next
				<< _GetStackWriter()->FormatHighlight();
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
				<< (int)Actual << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
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
				<< Value << _GetStackWriter()->FormatHighlight();

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
			<< (int)State << _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetStackWriter()->FormatHighlight();


	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->ClassConfigName
			<< _GetStackWriter()->FormatHighlight();

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::SkipWrite()
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetStackWriter()->FormatHighlight();

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
			<< (int)State << _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructRoot(const FName& Name)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DC_FAIL(DcDReadWrite, StructNameMismatch)
				<< StructClass->GetFName() << Name
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructEnd(const FName& Name)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		return DcExpect(Name == StructClass->GetFName(), [=] {
			return DC_FAIL(DcDReadWrite, StructNameMismatch)
				<< StructClass->GetFName() << Name
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
				<< (int)EDcDataEntry::ClassRoot << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectReference)
	{
		return DcExpect(Next == EDcDataEntry::ObjectReference
			|| Next == EDcDataEntry::Nil, [=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::ObjectReference << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectEnd)
	{
		return DcExpect(Next == EDcDataEntry::ClassEnd, [=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::ClassEnd << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectExpandKeyOrEnd)
	{
		return DcExpect(Next == EDcDataEntry::ClassEnd || Next == EDcDataEntry::Name,
			[=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
				<< (int)EDcDataEntry::ClassEnd << (int)EDcDataEntry::Name << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectExpandValue)
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
				<< (int)Actual << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectExpandKeyOrEnd)
	{
		Datum.Property = NextPropertyByName(Class->PropertyLink, Value);
		if (Datum.Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
				<< Value.ToString() << _GetStackWriter()->FormatHighlight();

		State = EState::ExpectExpandValue;
		return DcOk();
	}
	else if (State == EState::ExpectExpandValue)
	{
		DC_TRY((WriteValue<UNameProperty, FName>(*this, Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< _GetStackWriter()->FormatHighlight();

	if (!Datum.Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->ClassConfigName << Datum.Property->GetFName() << Datum.Property->GetClass()->GetFName()
			<< _GetStackWriter()->FormatHighlight();

	UProperty* Property = CastChecked<UProperty>(Datum.Property);

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(Datum.DataPtr);

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< _GetStackWriter()->FormatHighlight();

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::PeekWriteProperty(UField** OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		*OutProperty = Class;
		return DcOk();
	}
	else if (State == EState::ExpectExpandValue)
	{
		*OutProperty = Datum.Property;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassRoot(const FDcObjectPropertyStat& ClassStat)
{
	if (State == EState::ExpectRoot)
	{
		//	TODO may pass in derived class already
		//TRY(Expect(ClassStat.Name == Class->GetFName()));
		if (ClassStat.Reference == EDcObjectPropertyControl::ExternalReference)
		{
			State = EState::ExpectReference;
		}
		else if (ClassStat.Reference == EDcObjectPropertyControl::ExpandObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				//	expand a reference into a root
				Type = EType::Root;

				UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
				ClassObject = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);
				Datum.DataPtr = ClassObject;
				Datum.Property = ObjProperty->PropertyClass;

				if (!Datum.DataPtr)
				{
					//	inline object needs to be created by user
					return DC_FAIL(DcDReadWrite, WriteClassInlineNotCreated)
						<< ObjProperty->GetFName() << ObjProperty->GetClass()->GetFName()
						<< _GetStackWriter()->FormatHighlight();
				}
			}

			State = EState::ExpectExpandKeyOrEnd;
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
			<< (int)EState::ExpectRoot << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassEnd(const FDcObjectPropertyStat& ClassStat)
{
	if (State == EState::ExpectExpandKeyOrEnd
		|| State == EState::ExpectEnd)
	{
		State = EState::Ended;
		//	TODO now may pass in derived class
		//return Expect(ClassStat.Name == Class->GetFName());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandKeyOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteNil()
{
	if (State == EState::ExpectReference)
	{
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, nullptr);

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteObjectReference(const UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		//	`UObjectProperty::SetObjectPropertyValue` not taking const pointer 
		ClassObject = const_cast<UObject*>(Value);
		Datum.CastChecked<UObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, const_cast<UObject*>(Value));

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

void FDcWriteStateClass::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatClass(
		OutSegments,
		SegType,
		ObjectName,
		Class,
		State == EState::ExpectExpandKeyOrEnd || State == EState::ExpectExpandValue
			? Datum.Cast<UProperty>() 
			: nullptr
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
				<< (int)EDcDataEntry::MapRoot << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return DcExpect(Next == EDcDataEntry::MapEnd || Next == PropertyToDataEntry(MapProperty->KeyProp),
			[=] {
			return DC_FAIL(DcDReadWrite, DataTypeMismatch2)
				<< (int)EDcDataEntry::ClassEnd << (int)EDcDataEntry::Name << (int)Next
				<< _GetStackWriter()->FormatHighlight();
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
				<< (int)Actual << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectRoot << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectKeyOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
				<< (int)EDcDataEntry::ArrayRoot << (int)Next
				<< _GetStackWriter()->FormatHighlight();
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
					<< (int)Actual << (int)Next
					<< _GetStackWriter()->FormatHighlight();
			}
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

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
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

	++Index;
	return DcOk();
}

FDcResult FDcWriteStateArray::PeekWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

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
			<< (int)EState::ExpectRoot << (int)State
			<< _GetStackWriter()->FormatHighlight();
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
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

void FDcWriteStateArray::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatArray(OutSegments, SegType, ArrayProperty, Index,
		State == EState::ExpectItemOrEnd);
}

EDcPropertyWriteType FDcWriteStateSet::GetType()
{
	return EDcPropertyWriteType::SetProperty;
}

FDcResult FDcWriteStateSet::Peek(EDcDataEntry Next)
{
	if (State == EState::ExpectRoot)
	{
		return DcExpect(Next == EDcDataEntry::SetRoot, [=]{
			return DC_FAIL(DcDReadWrite, DataTypeMismatch)
				<< (int)EDcDataEntry::SetRoot << (int)Next
				<< _GetStackWriter()->FormatHighlight();
		});
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		if (Next == EDcDataEntry::SetEnd)
		{
			return DcOk();
		}
		else
		{
			EDcDataEntry Actual = PropertyToDataEntry(SetProperty->ElementProp);
			if (Next == Actual)
			{
				return DcOk();
			}
			else
			{
				return DC_FAIL(DcDReadWrite, DataTypeMismatch)
					<< (int)Actual << (int)Next
					<< _GetStackWriter()->FormatHighlight();
			}
		}
	}
	else if (State == EState::Ended)
	{
		return DC_FAIL(DcDReadWrite, AlreadyEnded)
			<< _GetStackWriter()->FormatHighlight();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcWriteStateSet::WriteName(const FName& Value)
{
	return WriteValue<UNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateSet::WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

	FScriptSetHelper SetHelper(SetProperty, SetPtr);
	SetHelper.AddUninitializedValue();
	OutDatum.Property = SetProperty->ElementProp;
	OutDatum.DataPtr = SetHelper.GetElementPtr(Index);

	bNeedsRehash = true;
	++Index;
	return DcOk();
}

FDcResult FDcWriteStateSet::SkipWrite()
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

	++Index;
	return DcOk();
}

FDcResult FDcWriteStateSet::PeekWriteProperty(UField** OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();

	*OutProperty = SetProperty->ElementProp;
	return DcOk();
}

FDcResult FDcWriteStateSet::WriteSetRoot()
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectItemOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateSet::WriteSetEnd()
{
	if (State == EState::ExpectItemOrEnd)
	{
		State = EState::Ended;

		if (bNeedsRehash)
		{
			FScriptSetHelper SetHelper(SetProperty, SetPtr);
			SetHelper.Rehash();
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetStackWriter()->FormatHighlight();
	}
}

void FDcWriteStateSet::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatSet(OutSegments, SegType, SetProperty, Index,
		State == EState::ExpectItemOrEnd);
}
