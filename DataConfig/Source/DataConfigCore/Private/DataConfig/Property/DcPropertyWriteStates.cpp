#include "DataConfig/Property/DcPropertyWriteStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/SerDe/DcSerDeCommon.h"

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

static FDcResult CheckExpectedProperty(FDcPropertyWriter* Parent, FField* Property, FFieldClass* ExpectedPropertyClass)
{
	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< ExpectedPropertyClass->GetFName() << Property->GetFName() << Property->GetClass()->GetFName()
			<< Parent->FormatHighlight();
	else
		return DcOk();
}

}	// namespace DcPropertyWriteStatesDetails

FDcResult FDcBaseWriteState::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry, bool*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteName(FDcPropertyWriter* Parent, const FName&){ return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass*, FDcPropertyDatum&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::SkipWrite(FDcPropertyWriter* Parent) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseWriteState::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant*) { return DC_FAIL(DcDCommon, NotImplemented); }

EDcPropertyWriteType FDcWriteStateNil::GetType()
{
	return EDcPropertyWriteType::Nil;
}

FDcResult FDcWriteStateNil::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
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

FDcResult FDcWriteStateStruct::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
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
		if (DcPropertyUtils::IsScalarArray(Property))
		{
			return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayRoot
				||  Next == EDcDataEntry::Blob); 
		}
		else
		{
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Property);
			return ReadOutOk(bOutOk, Next == Actual
				|| DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
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

FDcResult FDcWriteStateStruct::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		Property = Parent->Config.NextProcessPropertyByName(StructClass, Property, Value);
		if (Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
				<< Value << Parent->FormatHighlight();

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		DC_TRY((WriteValue<FNameProperty, FName>(Parent, *this, Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< Parent->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, Property, ExpectedPropertyClass));

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< Parent->FormatHighlight();

	State = EState::ExpectKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateStruct::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
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
			<< (int)State << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructRootAccess(FDcPropertyWriter* Parent, FDcStructAccess& Access)
{
	if (State == EState::ExpectRoot)
	{
		State = EState::ExpectKeyOrEnd;
		if (Access.Flag & FDcStructAccess::WriteCheckName)
		{
			return DcExpect(Access.Name == StructClass->GetFName(), [=] {
				return DC_FAIL(DcDReadWrite, StructNameMismatch)
					<< StructClass->GetFName() << Access.Name
					<< Parent->FormatHighlight();
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateStruct::WriteStructEndAccess(FDcPropertyWriter* Parent, FDcStructAccess& Access)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;
		if (Access.Flag & FDcStructAccess::WriteCheckName)
		{
			return DcExpect(Access.Name == StructClass->GetFName(), [=] {
				return DC_FAIL(DcDReadWrite, StructNameMismatch)
					<< StructClass->GetFName() << Access.Name
					<< Parent->FormatHighlight();
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
			<< Parent->FormatHighlight();
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

FDcResult FDcWriteStateClass::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
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
		FProperty* Property = Datum.CastFieldChecked<FProperty>();
		if (DcPropertyUtils::IsScalarArray(Property))
		{
			return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayRoot\
				|| Next == EDcDataEntry::Blob); 
		}
		else
		{
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Property);
			return ReadOutOk(bOutOk, Next == Actual
				|| DcPropertyWriteStatesDetails::CheckPropertyCoercion(Next, Actual));
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

FDcResult FDcWriteStateClass::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	if (State == EState::ExpectExpandKeyOrEnd)
	{
		Datum.Property =  Parent->Config.NextProcessPropertyByName(Class, Datum.CastField<FProperty>(), Value);
		Datum.DataPtr = nullptr;

		if (Datum.Property == nullptr)
			return DC_FAIL(DcDReadWrite, CantFindPropertyByName)
				<< Value << Parent->FormatHighlight();

		State = EState::ExpectExpandValue;
		return DcOk();
	}
	else if (State == EState::ExpectExpandValue)
	{
		DC_TRY((WriteValue<FNameProperty, FName>(Parent, *this, Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< Parent->FormatHighlight();

	FProperty* Property = Datum.CastFieldChecked<FProperty>();
	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, Property, ExpectedPropertyClass));

	OutDatum.Property = Property;
	OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(ClassObject);

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State != EState::ExpectExpandValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandValue << (int)State
			<< Parent->FormatHighlight();

	State = EState::ExpectExpandKeyOrEnd;
	return DcOk();
}

FDcResult FDcWriteStateClass::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
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
			<< (int)State << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassRootAccess(FDcPropertyWriter* Parent, FDcClassAccess& Access)
{
	if (State == EState::ExpectRoot)
	{
		if (Access.Flag & FDcClassAccess::WriteCheckName)
		{
			DC_TRY(DcExpect(Class->GetFName() == Access.Name, [=]{
				return DC_FAIL(DcDReadWrite, ClassNameMismatch)
					<< Class->GetFName() << Access.Name
					<< Parent->FormatHighlight();
			}));
		}

		if (Access.Control == FDcClassAccess::EControl::Default)
			Access.Control = ConfigControl;

		if (Access.Control == FDcClassAccess::EControl::ReferenceOrNil)
		{
			State = EState::ExpectReference;
		}
		else if (Access.Control == FDcClassAccess::EControl::ExpandObject)
		{
			if (Type == EType::PropertyNormalOrInstanced)
			{
				//	expand a reference into a root
				Type = EType::Root;
				
				FObjectProperty* ObjProperty = Datum.CastFieldChecked<FObjectProperty>();
				ClassObject = ObjProperty->GetObjectPropertyValue(Datum.DataPtr);

				DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(ClassObject));

				if (ClassObject == nullptr)
				{
					//	inline object needs to be created by user
					return DC_FAIL(DcDReadWrite, WriteClassInlineNotCreated)
						<< ObjProperty->GetFName() << ObjProperty->GetClass()->GetFName()
						<< Parent->FormatHighlight();
				}

				//	rewrite class as it might be a more derived
				Class = ClassObject->GetClass();
				Datum.Reset();
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteClassEndAccess(FDcPropertyWriter* Parent, FDcClassAccess& Access)
{
	if (State == EState::ExpectExpandKeyOrEnd
		|| State == EState::ExpectEnd)
	{
		if (Access.Flag & FDcClassAccess::WriteCheckName)
		{
			DC_TRY(DcExpect(Class->GetFName() == Access.Name, [=] {
				return DC_FAIL(DcDReadWrite, ClassNameMismatch)
					<< Class->GetFName() << Access.Name
					<< Parent->FormatHighlight();
			}));
		}

		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectExpandKeyOrEnd << (int)State
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteNil(FDcPropertyWriter* Parent)
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateClass::WriteObjectReference(FDcPropertyWriter* Parent, const UObject* Value)
{
	if (State == EState::ExpectReference)
	{
		if (Value == nullptr)
			return DC_FAIL(DcDReadWrite, WriteObjectReferenceDoNotAcceptNull);

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
			<< Parent->FormatHighlight();
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

FDcResult FDcWriteStateMap::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::MapRoot);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::MapEnd || Next == DcPropertyUtils::PropertyToDataEntry(MapHelper.KeyProp));
	}
	else if (State == EState::ExpectValue)
	{
		EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(MapHelper.ValueProp);
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

FDcResult FDcWriteStateMap::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	return WriteValue<FNameProperty, FName>(Parent, *this, Value);
}

FDcResult FDcWriteStateMap::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		//	crucial to construct, future write is copy assignment
		MapHelper.AddDefaultValue_Invalid_NeedsRehash();
		bNeedsRehash = true;
		MapHelper.GetKeyPtr(Index);

		FProperty* KeyProperty = MapHelper.GetKeyProperty();
		DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, KeyProperty, ExpectedPropertyClass));

		OutDatum.Property = KeyProperty;
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FProperty* ValueProperty = MapHelper.GetValueProperty();
		DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, ValueProperty, ExpectedPropertyClass));

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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateMap::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		++Index;

		if (Index >= MapHelper.Num())
			return DC_FAIL(DcDReadWrite, SkipOutOfRange)
				<< MapHelper.Num() << Parent->FormatHighlight();

		State = EState::ExpectKeyOrEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateMap::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		if (MapProperty == nullptr)
			return DC_FAIL(DcDReadWrite, PeekNoContainerProperty) << TEXT("Map");
		return ReadOutOk(OutProperty, MapProperty);
	}
	else if (State == EState::ExpectKeyOrEnd)
	{
		return ReadOutOk(OutProperty, MapHelper.KeyProp);
	}
	else if (State == EState::ExpectValue)
	{
		return ReadOutOk(OutProperty, MapHelper.ValueProp);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< (int)EState::ExpectKeyOrEnd << (int)EState::ExpectValue << (int)State
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateMap::WriteMapRoot(FDcPropertyWriter* Parent)
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateMap::WriteMapEnd(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectKeyOrEnd)
	{
		State = EState::Ended;

		if (bNeedsRehash)
		{
			MapHelper.Rehash();
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectKeyOrEnd << (int)State
			<< Parent->FormatHighlight();
	}
}

void FDcWriteStateMap::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatMap(OutSegments, SegType, MapName, MapHelper.KeyProp, MapHelper.ValueProp, Index,
		State == EState::ExpectKeyOrEnd || State == EState::ExpectValue);
}

EDcPropertyWriteType FDcWriteStateArray::GetType()
{
	return EDcPropertyWriteType::ArrayProperty;
}

FDcResult FDcWriteStateArray::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
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
			auto& ArrayAccess = (DcSerDeCommon::FScriptArrayHelperAccess&)ArrayHelper;
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(ArrayAccess.InnerProperty);
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

FDcResult FDcWriteStateArray::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	return WriteValue<FNameProperty, FName>(Parent, *this, Value);
}

FDcResult FDcWriteStateArray::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< Parent->FormatHighlight();

	auto& ArrayAccess = (DcSerDeCommon::FScriptArrayHelperAccess&)ArrayHelper;
	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, ArrayAccess.InnerProperty, ExpectedPropertyClass));

	ArrayHelper.AddValue();
	OutDatum.Property = ArrayAccess.InnerProperty;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	++Index;
	return DcOk();
}

FDcResult FDcWriteStateArray::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< Parent->FormatHighlight();

	++Index;

	if (Index >= ArrayHelper.Num())
		return DC_FAIL(DcDReadWrite, SkipOutOfRange) 
			<< ArrayHelper.Num() << Parent->FormatHighlight();

	return DcOk();
}

FDcResult FDcWriteStateArray::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		if (ArrayProperty == nullptr)
			return DC_FAIL(DcDReadWrite, PeekNoContainerProperty) << TEXT("Array");
		return ReadOutOk(OutProperty, ArrayProperty);
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		auto& ArrayAccess = (DcSerDeCommon::FScriptArrayHelperAccess&)ArrayHelper;
		return ReadOutOk(OutProperty, ArrayAccess.InnerProperty);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)EState::ExpectItemOrEnd << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateArray::WriteArrayRoot(FDcPropertyWriter* Parent)
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateArray::WriteArrayEnd(FDcPropertyWriter* Parent)
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
			<< Parent->FormatHighlight();
	}
}

void FDcWriteStateArray::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	auto& ArrayAccess = (DcSerDeCommon::FScriptArrayHelperAccess&)ArrayHelper;
	DcPropertyHighlight::FormatArray(OutSegments, SegType, ArrayName, ArrayAccess.InnerProperty, Index,
		State == EState::ExpectItemOrEnd);
}

EDcPropertyWriteType FDcWriteStateSet::GetType()
{
	return EDcPropertyWriteType::SetProperty;
}

FDcResult FDcWriteStateSet::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
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
			EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(SetHelper.ElementProp);
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

FDcResult FDcWriteStateSet::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	return WriteValue<FNameProperty, FName>(Parent, *this, Value);
}

FDcResult FDcWriteStateSet::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< Parent->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, SetHelper.ElementProp, ExpectedPropertyClass));

	SetHelper.AddDefaultValue_Invalid_NeedsRehash();
	OutDatum.Property = SetHelper.ElementProp;
	OutDatum.DataPtr = SetHelper.GetElementPtr(Index);

	bNeedsRehash = true;
	++Index;
	return DcOk();
}

FDcResult FDcWriteStateSet::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State != EState::ExpectItemOrEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< Parent->FormatHighlight();

	++Index;

	if (Index >= SetHelper.Num())
		return DC_FAIL(DcDReadWrite, SkipOutOfRange) 
			<< SetHelper.Num() << Parent->FormatHighlight();

	return DcOk();
}

FDcResult FDcWriteStateSet::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
{
	if (State == EState::ExpectRoot)
	{
		if (SetProperty == nullptr)
			return DC_FAIL(DcDReadWrite, PeekNoContainerProperty) << TEXT("Set");
		return ReadOutOk(OutProperty, SetProperty);
	}
	else if (State == EState::ExpectItemOrEnd)
	{
		return ReadOutOk(OutProperty, SetHelper.ElementProp);
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)EState::ExpectItemOrEnd << Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateSet::WriteSetRoot(FDcPropertyWriter* Parent)
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
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateSet::WriteSetEnd(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectItemOrEnd)
	{
		State = EState::Ended;

		if (bNeedsRehash)
		{
			SetHelper.Rehash();
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItemOrEnd << (int)State
			<< Parent->FormatHighlight();
	}
}

void FDcWriteStateSet::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatSet(OutSegments, SegType, SetName, SetHelper.ElementProp, Index,
		State == EState::ExpectItemOrEnd);
}

EDcPropertyWriteType FDcWriteStateScalar::GetType()
{
	return EDcPropertyWriteType::ScalarProperty;
}

FDcResult FDcWriteStateScalar::PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk)
{
	if (State == EState::ExpectScalar
		|| State == EState::ExpectArrayItem)
	{
		return ReadOutOk(bOutOk, Next == DcPropertyUtils::PropertyToDataEntry(ScalarField));
	}
	else if (State == EState::ExpectArrayRoot)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayRoot);
	}
	else if (State == EState::ExpectArrayEnd)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayEnd);
	}
	else
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended);
	}
}

FDcResult FDcWriteStateScalar::WriteName(FDcPropertyWriter* Parent, const FName& Value)
{
	return WriteValue<FNameProperty, FName>(Parent, *this, Value);
}

FDcResult FDcWriteStateScalar::WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectScalar
		&& State != EState::ExpectArrayItem)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectScalar << State
			<< Parent->FormatHighlight();

	DC_TRY(DcPropertyWriteStatesDetails::CheckExpectedProperty(Parent, ScalarField, ExpectedPropertyClass));

	if (State == EState::ExpectScalar)
	{
		OutDatum.Property = ScalarField;
		OutDatum.DataPtr = ScalarPtr;
		State = EState::Ended;
	}
	else
	{
		OutDatum.Property = ScalarField;
		OutDatum.DataPtr = (uint8*)ScalarPtr + (PTRINT)(ScalarField->ElementSize * Index);

		++Index;
		if (Index == ScalarField->ArrayDim)
			State = EState::ExpectArrayEnd;
	}

	return DcOk();
}

FDcResult FDcWriteStateScalar::SkipWrite(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectScalar)
	{
		State = EState::Ended;
		return DcOk();
	}
	else if (State == EState::ExpectArrayItem)
	{
		++Index;
		if (Index == ScalarField->ArrayDim)
			State = EState::ExpectArrayEnd;

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< EState::ExpectScalar << EState::ExpectArrayItem << State
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateScalar::PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty)
{
	if (State != EState::ExpectScalar
		&& State != EState::ExpectArrayItem)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< EState::ExpectScalar << EState::ExpectArrayItem << State
			<< Parent->FormatHighlight();

	return ReadOutOk(OutProperty, ScalarField);
}

FDcResult FDcWriteStateScalar::WriteArrayRoot(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectArrayRoot)
	{
		//	ArrayDim > 1 so there's at least some item to read
		State = EState::ExpectArrayItem;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectArrayRoot << State
			<< Parent->FormatHighlight();
	}
}

FDcResult FDcWriteStateScalar::WriteArrayEnd(FDcPropertyWriter* Parent)
{
	if (State == EState::ExpectArrayEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectArrayEnd << State
			<< Parent->FormatHighlight();
	}
}

void FDcWriteStateScalar::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatScalar(OutSegments, SegType, ScalarField, Index, State == EState::ExpectArrayItem);
}
