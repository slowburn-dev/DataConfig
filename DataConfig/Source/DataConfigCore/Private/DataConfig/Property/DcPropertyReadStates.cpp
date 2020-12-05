#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Property/DcPropertyReader.h"

static FDcPropertyReader* _GetPropertyReader() 
{
	return (FDcPropertyReader*)(DcEnv().ReaderStack.Top());
}

namespace DcPropertyReadStateDetails
{

static FDcResult CheckExpectedProperty(UProperty* Property, UClass* ExpectedPropertyClass)
{
	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
		<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->ClassConfigName
		<< _GetPropertyReader()->FormatHighlight();
	else
		return DcOk();
}

}	// namespace DcPropertyReadStateDetails


FDcResult FDcBaseReadState::PeekRead(EDcDataEntry* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseReadState::ReadName(FName* OutNamePtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseReadState::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcBaseReadState::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	checkNoEntry();
}

EDcPropertyReadType FDcReadStateNil::GetType()
{
	return EDcPropertyReadType::Nil;
}

FDcResult FDcReadStateNil::PeekRead(EDcDataEntry* OutPtr)
{
	ReadOut(OutPtr, EDcDataEntry::Ended);
	return DcOk();
}

void FDcReadStateNil::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatNil(OutSegments, SegType);
}

EDcPropertyReadType FDcReadStateClass::GetType()
{
	return EDcPropertyReadType::ClassProperty;
}

FDcResult FDcReadStateClass::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		ReadOut(OutPtr, EDcDataEntry::ClassRoot);
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		ReadOut(OutPtr,EDcDataEntry::ClassEnd);
		return DcOk();
	}
	else if (State == EState::ExpectNil)
	{
		ReadOut(OutPtr, EDcDataEntry::Nil);
		return DcOk();
	}
	else if (State == EState::ExpectReference)
	{
		ReadOut(OutPtr, EDcDataEntry::ObjectReference);
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		ReadOut(OutPtr, EDcDataEntry::Name);
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		ReadOut(OutPtr, PropertyToDataEntry(Property));
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcReadStateClass::ReadName(FName* OutNamePtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		ReadOut(OutNamePtr, Property->GetFName());

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FDcPropertyDatum Datum;
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

		ReadOut(OutNamePtr, Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		// should've be already handled in `ReadName`
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);

		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(Property, ExpectedPropertyClass));

		OutDatum.Property = Property;
		OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(ClassObject);

		EndReadValue();
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

void FDcReadStateClass::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatClass(OutSegments, SegType, ObjectName, Class, Property);
}

FDcResult FDcReadStateClass::EndReadValue()
{
	if (State == EState::ExpectValue)
	{
		Property = NextEffectiveProperty(Property);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadClassRoot(FDcObjectPropertyStat* OutClassPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassObject == nullptr)
		{
			check(Type == EType::PropertyNormal || Type == EType::PropertyInstanced);
			if (OutClassPtr)
			{
				OutClassPtr->Name = Class->GetFName();
				OutClassPtr->Reference = EDcObjectPropertyControl::ExternalReference;
			}

			State = EState::ExpectNil;
			return DcOk();
		}
		else
		{
			if (Type == EType::PropertyNormal)
			{
				State = EState::ExpectReference;

				if (OutClassPtr)
				{
					OutClassPtr->Name = Class->GetFName();
					OutClassPtr->Reference = EDcObjectPropertyControl::ExternalReference;
				}

				return DcOk();
			}
			else if (Type == EType::PropertyInstanced
				|| Type == EType::Root)
			{
				check(ClassObject);
				UClass* Cls = ClassObject->GetClass();
				if (OutClassPtr)
				{
					OutClassPtr->Name = Cls->GetFName();
					OutClassPtr->Reference = EDcObjectPropertyControl::ExpandObject;
				}

				Property = FirstEffectiveProperty(Cls->PropertyLink);
				if (Property == nullptr)
				{
					State = EState::ExpectEnd;
				}
				else
				{
					State = EState::ExpectKey;
				}
				return DcOk();
			}
			else
			{
				return DcNoEntry();
			}
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadClassEnd(FDcObjectPropertyStat* OutClassPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutClassPtr)
		{
			OutClassPtr->Name = Class->GetFName();
			OutClassPtr->Reference = Type == EType::Root
				? EDcObjectPropertyControl::ExpandObject
				: EDcObjectPropertyControl::ExternalReference;
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadNil()
{
	if (State == EState::ExpectNil)
	{
		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectNil << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadObjectReference(UObject** OutPtr)
{
	if (State == EState::ExpectReference)
	{
		ReadOut(OutPtr, ClassObject);

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

EDcPropertyReadType FDcReadStateStruct::GetType()
{
	return EDcPropertyReadType::StructProperty;
}

FDcResult FDcReadStateStruct::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		ReadOut(OutPtr, EDcDataEntry::StructRoot);
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		ReadOut(OutPtr, EDcDataEntry::StructEnd);
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		ReadOut(OutPtr, EDcDataEntry::Name);
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		ReadOut(OutPtr, PropertyToDataEntry(Property));
		return DcOk();
	}
	else if (State == EState::Ended)
	{
		ReadOut(OutPtr, EDcDataEntry::Ended);
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcReadStateStruct::ReadName(FName* OutNamePtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		ReadOut(OutNamePtr, Property->GetFName());

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FDcPropertyDatum Datum;
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

		ReadOut(OutNamePtr, Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		// should've be already handled in `ReadName`
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);

		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(Property, ExpectedPropertyClass));

		OutDatum.Property = Property;
		OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

		EndReadValue();	// this now guarentee success
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

void FDcReadStateStruct::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatStruct(OutSegments, SegType, StructName, StructClass, Property);
}

FDcResult FDcReadStateStruct::EndReadValue()
{
	if (State == EState::ExpectValue)
	{
		Property = NextEffectiveProperty(Property);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateStruct::ReadStructRoot(FName* OutNamePtr)
{
	if (State == EState::ExpectRoot)
	{
		if (OutNamePtr)
			*OutNamePtr = StructClass->GetFName();

		Property = FirstEffectiveProperty(StructClass->PropertyLink);
		if (Property == nullptr)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateStruct::ReadStructEnd(FName* OutNamePtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutNamePtr)
		{
			*OutNamePtr = StructClass->GetFName();
		}

		//	!!! note that this doens't need to EndRead, it's done before entering this state
		//TRY(GetTopState(Self).EndReadValue());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

EDcPropertyReadType FDcReadStateMap::GetType()
{
	return EDcPropertyReadType::MapProperty;
}

FDcResult FDcReadStateMap::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		ReadOut(OutPtr, EDcDataEntry::MapRoot);
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		ReadOut(OutPtr, EDcDataEntry::MapEnd);
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		check(MapProperty);
		ReadOut(OutPtr, PropertyToDataEntry(MapProperty->KeyProp));
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		ReadOut(OutPtr, PropertyToDataEntry(MapProperty->ValueProp));
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcReadStateMap::ReadName(FName* OutNamePtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr));
	return DcOk();
}

FDcResult FDcReadStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();

	FScriptMapHelper MapHelper(MapProperty, MapPtr);
	if (State == EState::ExpectKey)
	{
		UProperty* KeyProperty = MapHelper.GetKeyProperty();
		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(KeyProperty, ExpectedPropertyClass));

		OutDatum.Property = KeyProperty;
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);
	}
	else if (State == EState::ExpectValue)
	{
		UProperty* ValueProperty = MapHelper.GetValueProperty();
		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(ValueProperty, ExpectedPropertyClass));

		OutDatum.Property = ValueProperty;
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);
	}
	else
	{
		return DcNoEntry();
	}

	DC_TRY(EndReadValue());
	return DcOk();
}

void FDcReadStateMap::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatMap(OutSegments, SegType, MapProperty, Index,
		State == EState::ExpectKey || State == EState::ExpectValue);
}

FDcResult FDcReadStateMap::EndReadValue()
{
	if (State == EState::ExpectKey)
	{
		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMap* ScriptMap = (FScriptMap*)MapPtr;
		Index += 1;
		if (Index < ScriptMap->Num())
		{
			State = EState::ExpectKey;
		}
		else
		{
			State = EState::ExpectEnd;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateMap::ReadMapRoot()
{
	if (State == EState::ExpectRoot)
	{
		//	check map effectiveness
		if (!IsEffectiveProperty(MapProperty->KeyProp)
			|| !IsEffectiveProperty(MapProperty->ValueProp))
		{
			State = EState::ExpectEnd;
			return DcOk();
		}

		FScriptMap* ScriptMap = (FScriptMap*)MapPtr;
		if (ScriptMap->Num() == 0)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectKey;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateMap::ReadMapEnd()
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

EDcPropertyReadType FDcReadStateArray::GetType()
{
	return EDcPropertyReadType::ArrayProperty;
}

FDcResult FDcReadStateArray::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		ReadOut(OutPtr, EDcDataEntry::ArrayRoot);
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		ReadOut(OutPtr, EDcDataEntry::ArrayEnd);
		return DcOk();
	}
	else if (State == EState::ExpectItem)
	{
		check(ArrayProperty->Inner);
		ReadOut(OutPtr, PropertyToDataEntry(ArrayProperty->Inner));
		return DcOk();
	}
	else if (State == EState::Ended)
	{
		ReadOut(OutPtr, EDcDataEntry::Ended);
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcReadStateArray::ReadName(FName* OutNamePtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr));
	return DcOk();
}

FDcResult FDcReadStateArray::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();

	check(State == EState::ExpectItem);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);

	DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(ArrayProperty->Inner, ExpectedPropertyClass));

	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	DC_TRY(EndReadValue());
	return DcOk();
}

void FDcReadStateArray::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatArray(OutSegments, SegType, ArrayProperty, Index,
		State == EState::ExpectItem);
}

FDcResult FDcReadStateArray::EndReadValue()
{
	if (State == EState::ExpectItem)
	{
		FScriptArray* ScriptArray = (FScriptArray*)ArrayPtr;
		Index += 1;
		if (Index < ScriptArray->Num())
		{
			State = EState::ExpectItem;
		}
		else
		{
			State = EState::ExpectEnd;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectItem << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateArray::ReadArrayRoot()
{
	if (State == EState::ExpectRoot)
	{
		if (!IsEffectiveProperty(ArrayProperty->Inner))
		{
			State = EState::ExpectEnd;
			return DcOk();
		}

		FScriptArray* ScriptArray = (FScriptArray*)ArrayPtr;
		if (ScriptArray->Num() == 0)
		{
			State = EState::ExpectEnd;
		}
		else
		{
			State = EState::ExpectItem;
		}
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateArray::ReadArrayEnd()
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

EDcPropertyReadType FDcReadStateSet::GetType()
{
	return FDcReadStateSet::ID;
}

FDcResult FDcReadStateSet::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		ReadOut(OutPtr, EDcDataEntry::SetRoot);
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		ReadOut(OutPtr, EDcDataEntry::SetEnd);
		return DcOk();
	}
	else if (State == EState::ExpectItem)
	{
		check(SetProperty->ElementProp);
		ReadOut(OutPtr, PropertyToDataEntry(SetProperty->ElementProp));
		return DcOk();
	}
	else if (State == EState::Ended)
	{
		ReadOut(OutPtr, EDcDataEntry::Ended);
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

FDcResult FDcReadStateSet::ReadName(FName* OutNamePtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr));

	return DcOk();
}

FDcResult FDcReadStateSet::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItem)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();

	check(State == EState::ExpectItem);
	FScriptSetHelper SetHelper(SetProperty, SetPtr);

	DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(SetProperty->ElementProp, ExpectedPropertyClass));

	OutDatum.Property = SetProperty->ElementProp;
	OutDatum.DataPtr = SetHelper.GetElementPtr(Index);

	if (++Index == SetHelper.Num())
		State = EState::ExpectEnd;

	return DcOk();
}

void FDcReadStateSet::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatSet(OutSegments, SegType, SetProperty, Index,
		State == EState::ExpectItem);
}

FDcResult FDcReadStateSet::ReadSetRoot()
{
	if (State == EState::ExpectRoot)
	{
		if (!IsEffectiveProperty(SetProperty->ElementProp))
		{
			State = EState::ExpectEnd;
			return DcOk();
		}

		FScriptSet* ScriptSet = (FScriptSet*)SetPtr;
		State = ScriptSet->Num() == 0 ? EState::ExpectEnd : EState::ExpectItem;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateSet::ReadSetEnd()
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State
			<< _GetPropertyReader()->FormatHighlight();
	}
}
