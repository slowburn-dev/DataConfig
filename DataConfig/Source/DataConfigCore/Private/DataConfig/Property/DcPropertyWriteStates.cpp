#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"

static FDcPropertyWriter* _GetPropertyWriter()
{
	return (FDcPropertyWriter*)(DcEnv().WriterStack.Top());
}

namespace DcPropertyWriteStatesDetails
{

static bool CheckPropertyCoercion(EDcDataEntry Next, EDcDataEntry Actual)
{
	if (Next == EDcDataEntry::Blob && Actual == EDcDataEntry::ArrayRoot)
	{
		return true;	//	write array as blob
	}
	else if (Next == EDcDataEntry::Blob && Actual == EDcDataEntry::StructRoot)
	{
		return true;	//	write struct as blob
	}
	else
	{
		return false;
	}
}

static FDcResult CheckExpectedProperty(FField* Property, FFieldClass* ExpectedPropertyClass)
{
	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->GetFName() << Property->GetFName() << Property->GetClass()->GetFName()
			<< _GetPropertyWriter()->FormatHighlight();
	else
		return DcOk();
}

}	// namespace DcPropertyWriteStatesDetails

FDcResult FDcBaseWriteState::PeekWrite(EDcDataEntry, bool*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteName(const FName&){ return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteDataEntry(FFieldClass*, FDcPropertyDatum&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::SkipWrite() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::PeekWriteProperty(FFieldVariant*) { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcBaseWriteState::FormatHighlightSegment(TArray<FString>&, DcPropertyHighlight::EFormatSeg)
{
	checkNoEntry();
}

EDcPropertyWriteType FDcWriteStateNil::GetType()
{
	return EDcPropertyWriteType::Nil;
}

FDcResult FDcWriteStateNil::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
}

void FDcWriteStateNil::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatNil(OutSegments, SegType);
}

EDcPropertyWriteType FDcWriteStateStruct::GetType()
{
	return EDcPropertyWriteType::StructProperty;
}

FDcResult FDcWriteStateStruct::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::StructRoot);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::StructEnd || Next == EDcDataEntry::Name);
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Property);

		return ReadOutOk(bOutOk, Next == Actual || DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
	}
	else if (State == EState::Ended)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcWriteStateStruct::WriteName(const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = _GetPropertyWriter()->Config.NextProcessPropertyByName(StructClass, Property, Value);
		if (Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
				<< Value << _GetPropertyWriter()->FormatHighlight();

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		DC_TRY((WriteValue<FNameProperty, FName>(*this, Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();


	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Property, ExpectedPropertyClass));

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
			<< _GetPropertyWriter()->FormatHighlight();

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(OutProperty, StructClass);
	}
	else if (State == EState::ExpectValue)
	{
		return ReadOutOk(OutProperty, Property);

	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructRoot(const FDcStructStat& Struct)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		if (Struct.Flag & FDcStructStat::WriteCheckName)
		{
			return DcExpect(Struct.Name == StructClass->GetFName(), [=] {
				return DC_FAIL(DcDReadWrite, StructNameMismatch)
					<< StructClass->GetFName() << Struct.Name
					<< _GetPropertyWriter()->FormatHighlight();
			});
		}
		else
		{
			return DcOk();
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructEnd(const FDcStructStat& Struct)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		if (Struct.Flag & FDcStructStat::WriteCheckName)
		{
			return DcExpect(Struct.Name == StructClass->GetFName(), [=] {
				return DC_FAIL(DcDReadWrite, StructNameMismatch)
					<< StructClass->GetFName() << Struct.Name
					<< _GetPropertyWriter()->FormatHighlight();
			});
		}
		else
		{
			return DcOk();
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
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

FDcResult FDcWriteStateClass::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ClassRoot);
	}
	else if (State == EState::ExpectReference)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ObjectReference || Next == EDcDataEntry::Nil);
	}
	else if (State == EState::ExpectEnd)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ClassEnd);
	}
	else if (State == EState::ExpectExpandKeyOrEnd)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ClassEnd || Next == EDcDataEntry::Name);
	}
	else if (State == EState::ExpectExpandValue)
	{
		check(!Datum.IsNone());
		EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Datum.CastFieldChecked());
		return ReadOutOk(bOutOk, Next == Actual || DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
	}
	else if (State == EState::Ended)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcWriteStateClass::WriteName(const FName& Value)
{
	if (State == EState::ExpectExpandKeyOrEnd)
	{
		Datum.Property =  _GetPropertyWriter()->Config.NextProcessPropertyByName(Class, Datum.CastField<FProperty>(), Value);
		Datum.DataPtr = nullptr;

		if (Datum.Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
				<< Value << _GetPropertyWriter()->FormatHighlight();

		State = EState::ExpectExpandValue;
		return DcOk();
	}
	else if (State == EState::ExpectExpandValue)
	{
		DC_TRY((WriteValue<FNameProperty, FName>(*this, Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	FProperty* Property = Datum.CastFieldChecked<FProperty>();
	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Property, ExpectedPropertyClass));

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(ClassObject);

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::SkipWrite()
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(OutProperty, Class);
	}
	else if (State == EState::ExpectExpandValue)
	{
		return ReadOutOk(OutProperty, Datum.Property);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassRoot(const FDcClassStat& ClassStat)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassStat.Flag & FDcClassStat::WriteCheckName)
		{
			DC_TRY(DcExpect(Class->GetFName() == ClassStat.Name, [=]{
				return DC_FAIL(DcDReadWrite, ClassNameMismatch)
					<< Class->GetFName() << ClassStat.Name
					<< _GetPropertyWriter()->FormatHighlight();
			}));
		}

		if (ClassStat.Control == FDcClassStat::EControl::ReferenceOrNil)
		{
			State = EState::ExpectReference;
		}
		else if (ClassStat.Control == FDcClassStat::EControl::ExpandObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				//	expand a reference into a root
				Type = EType::Root;

				FObjectProperty* ObjProperty = Datum.CastFieldChecked<FObjectProperty>();
				ClassObject = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);
				//	rewrite class as it might be a more derived
				Class = ClassObject->GetClass();
				Datum.Reset();

				if (!ClassObject)
				{
					//	inline object needs to be created by user
					return DC_FAIL(DcDReadWrite, WriteClassInlineNotCreated)
						<< ObjProperty->GetFName() << ObjProperty->GetClass()->GetFName()
						<< _GetPropertyWriter()->FormatHighlight();
				}
			}

			State = EState::ExpectExpandKeyOrEnd;
		}
		else
		{
			return DcNoEntry();
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassEnd(const FDcClassStat& ClassStat)
{
	if (State == EState::ExpectExpandKeyOrEnd
		|| State == EState::ExpectEnd)
	{
		if (ClassStat.Flag & FDcClassStat::WriteCheckName)
		{
			DC_TRY(DcExpect(Class->GetFName() == ClassStat.Name, [=] {
				return DC_FAIL(DcDReadWrite, ClassNameMismatch)
					<< Class->GetFName() << ClassStat.Name
					<< _GetPropertyWriter()->FormatHighlight();
			}));
		}

		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandKeyOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteNil()
{
	if (State == EState::ExpectReference)
	{
		Datum.CastFieldChecked<FObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, nullptr);

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteObjectReference(const UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		//	`FObjectProperty::SetObjectPropertyValue` not taking const pointer
		ClassObject = const_cast<UObject*>(Value);
		Datum.CastFieldChecked<FObjectProperty>()->SetObjectPropertyValue(Datum.DataPtr, const_cast<UObject*>(Value));

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
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
			? Datum.CastField<FProperty>()
			: nullptr
	);
}

EDcPropertyWriteType FDcWriteStateMap::GetType()
{
	return EDcPropertyWriteType::MapProperty;
}

FDcResult FDcWriteStateMap::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::MapRoot);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		check(MapProperty);
		return ReadOutOk(bOutOk, Next == EDcDataEntry::MapEnd || Next == DcPropertyUtils::PropertyToDataEntry(MapProperty->KeyProp));
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(MapProperty->ValueProp);
		return ReadOutOk(bOutOk, Next == Actual || DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
	}
	else if (State == EState::Ended)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcWriteStateMap::WriteName(const FName& Value)
{
	return WriteValue<FNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateMap::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		//	crucial to construct, future write is copy assignment
		MapHelper.AddDefaultValue_Invalid_NeedsRehash();
		bNeedsRehash = true;
		MapHelper.GetKeyPtr(Index);

		FProperty* KeyProperty = MapHelper.GetKeyProperty();
		DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(KeyProperty, ExpectedPropertyClass));

		OutDatum.Property = KeyProperty;
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);

		FProperty* ValueProperty = MapHelper.GetValueProperty();
		DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(ValueProperty, ExpectedPropertyClass));

		OutDatum.Property = ValueProperty;
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);

		++Index;
		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
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

		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		if (Index >= MapHelper.Num())
			return DC_FAIL(DcDReadWrite, SkipOutOfRange)
				<< MapHelper.Num() << _GetPropertyWriter()->FormatHighlight();

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateMap::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		return ReadOutOk(OutProperty, MapProperty->KeyProp);
	}
	else if (State == EState::ExpectValue)
	{
		return ReadOutOk(OutProperty, MapProperty->ValueProp);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< _GetPropertyWriter()->FormatHighlight();
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
			<< _GetPropertyWriter()->FormatHighlight();
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
			<< _GetPropertyWriter()->FormatHighlight();
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

FDcResult FDcWriteStateArray::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayRoot);
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		if (Next == EDcDataEntry::ArrayEnd)
		{
			return ReadOutOk(bOutOk, true);
		}
		else
		{
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(ArrayProperty->Inner);
			return ReadOutOk(bOutOk, Next == Actual || DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
		}
	}
	else if (State == EState::Ended)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcWriteStateArray::WriteName(const FName& Value)
{
	return WriteValue<FNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateArray::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(ArrayProperty->Inner, ExpectedPropertyClass));

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
			<< _GetPropertyWriter()->FormatHighlight();

	++Index;

	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	if (Index >= ArrayHelper.Num())
		return DC_FAIL(DcDReadWrite, SkipOutOfRange) 
			<< ArrayHelper.Num() << _GetPropertyWriter()->FormatHighlight();

	return DcOk();
}

FDcResult FDcWriteStateArray::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	return ReadOutOk(OutProperty, ArrayProperty->Inner);
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
			<< _GetPropertyWriter()->FormatHighlight();
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
			<< _GetPropertyWriter()->FormatHighlight();
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

FDcResult FDcWriteStateSet::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::SetRoot);
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		if (Next == EDcDataEntry::SetEnd)
		{
			return ReadOutOk(bOutOk, true);
		}
		else
		{
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(SetProperty->ElementProp);
			return ReadOutOk(bOutOk, Next == Actual || DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
		}
	}
	else if (State == EState::Ended)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcWriteStateSet::WriteName(const FName& Value)
{
	return WriteValue<FNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateSet::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(SetProperty->ElementProp, ExpectedPropertyClass));

	FScriptSetHelper SetHelper(SetProperty, SetPtr);
	SetHelper.AddDefaultValue_Invalid_NeedsRehash();
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
			<< _GetPropertyWriter()->FormatHighlight();

	++Index;

	FScriptSetHelper SetHelper(SetProperty, SetPtr);
	if (Index >= SetHelper.Num())
		return DC_FAIL(DcDReadWrite, SkipOutOfRange) 
			<< SetHelper.Num() << _GetPropertyWriter()->FormatHighlight();

	return DcOk();
}

FDcResult FDcWriteStateSet::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< _GetPropertyWriter()->FormatHighlight();

	return ReadOutOk(OutProperty, SetProperty->ElementProp);
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
			<< _GetPropertyWriter()->FormatHighlight();
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
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

void FDcWriteStateSet::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatSet(OutSegments, SegType, SetProperty, Index,
		State == EState::ExpectItemOrEnd);
}

EDcPropertyWriteType FDcWriteStateScalar::GetType()
{
	return EDcPropertyWriteType::ScalarProperty;
}

FDcResult FDcWriteStateScalar::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectWrite)
	{
		return ReadOutOk(bOutOk, Next == DcPropertyUtils::PropertyToDataEntry(ScalarField));
	}
	else
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
}

FDcResult FDcWriteStateScalar::WriteName(const FName& Value)
{
	return WriteValue<FNameProperty, FName>(*this, Value);
}

FDcResult FDcWriteStateScalar::WriteDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectWrite)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectWrite << State
			<< _GetPropertyWriter()->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(ScalarField, ExpectedPropertyClass));

	OutDatum.Property = ScalarField;
	OutDatum.DataPtr = ScalarPtr;

	State = EState::Ended;
	return DcOk();
}

FDcResult FDcWriteStateScalar::SkipWrite()
{
	if (State == EState::ExpectWrite)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectWrite << State
			<< _GetPropertyWriter()->FormatHighlight();
	}
}

FDcResult FDcWriteStateScalar::PeekWriteProperty(FFieldVariant* OutProperty)
{
	if (State != EState::ExpectWrite)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectWrite << State
			<< _GetPropertyWriter()->FormatHighlight();

	return ReadOutOk(OutProperty, ScalarField);
}

void FDcWriteStateScalar::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatScalar(OutSegments, SegType, ScalarField);
}
