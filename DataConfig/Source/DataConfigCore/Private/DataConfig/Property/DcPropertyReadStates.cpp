#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Property/DcPropertyReader.h"

static FDcPropertyReader* _GetPropertyReader()
{
	return (FDcPropertyReader*)(DcEnv().ReaderStack.Top());
}

namespace DcPropertyReadStateDetails
{

static FDcResult CheckExpectedProperty(FProperty* Property, FFieldClass* ExpectedPropertyClass)
{
	if (!Property->IsA(ExpectedPropertyClass))
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
		<< ExpectedPropertyClass->GetFName() << Property->GetFName() << Property->GetClass()->GetFName()
		<< _GetPropertyReader()->FormatHighlight();
	else
		return DcOk();
}

}	// namespace DcPropertyReadStateDetails


FDcResult FDcBaseReadState::PeekRead(EDcDataEntry*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseReadState::ReadName(FName*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseReadState::ReadDataEntry(FFieldClass*, FDcPropertyDatum&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcBaseReadState::SkipRead() { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcBaseReadState::FormatHighlightSegment(TArray<FString>&, DcPropertyHighlight::EFormatSeg)
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
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(Property));
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
		DC_TRY(ReadDataEntry(FNameProperty::StaticClass(), Datum));

		ReadOut(OutNamePtr, Datum.CastFieldChecked<FNameProperty>()->GetPropertyValue(Datum.DataPtr));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateClass::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
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

		EndValueRead();
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

FDcResult FDcReadStateClass::ReadClassRoot(FDcClassStat* OutClassPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassObject == nullptr)
		{
			check(Type == EType::PropertyNormal || Type == EType::PropertyInstanced);
			if (OutClassPtr)
			{
				OutClassPtr->Name = Class->GetFName();
				OutClassPtr->Control = FDcClassStat::EControl::ExternalReference;
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
					OutClassPtr->Control = FDcClassStat::EControl::ExternalReference;
				}

				return DcOk();
			}
			//	inline object read as struct, ie expanded
			else if (Type == EType::PropertyInstanced
				|| Type == EType::Root)
			{
				check(ClassObject);
				UClass* Cls = ClassObject->GetClass();
				if (OutClassPtr)
				{
					OutClassPtr->Name = Cls->GetFName();
					OutClassPtr->Control = FDcClassStat::EControl::ExpandObject;
				}

				Property = DcPropertyUtils::FirstEffectiveProperty(Cls->PropertyLink);
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

FDcResult FDcReadStateClass::ReadClassEnd(FDcClassStat* OutClassPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutClassPtr)
		{
			OutClassPtr->Name = Class->GetFName();
			OutClassPtr->Control = Type == EType::Root
				? FDcClassStat::EControl::ExpandObject
				: FDcClassStat::EControl::ExternalReference;
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

FDcResult FDcReadStateClass::SkipRead()
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
		<< EState::ExpectValue << State
		<< _GetPropertyReader()->FormatHighlight();

	EndValueRead();
	return DcOk();
}

void FDcReadStateClass::EndValueRead()
{
	check(State == EState::ExpectValue);
	Property = DcPropertyUtils::NextEffectiveProperty(Property);
	if (Property == nullptr)
		State = EState::ExpectEnd;
	else
		State = EState::ExpectKey;
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
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(Property));
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
		DC_TRY(ReadDataEntry(FNameProperty::StaticClass(), Datum));

		ReadOut(OutNamePtr, Datum.CastFieldChecked<FNameProperty>()->GetPropertyValue(Datum.DataPtr));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateStruct::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
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

		EndValueRead();
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

FDcResult FDcReadStateStruct::ReadStructRoot(FDcStructStat* OutStructPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (OutStructPtr)
		{
			OutStructPtr->Name = StructClass->GetFName();
		}

		Property = DcPropertyUtils::FirstEffectiveProperty(StructClass->PropertyLink);
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

FDcResult FDcReadStateStruct::ReadStructEnd(FDcStructStat* OutStructPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutStructPtr)
		{
			OutStructPtr->Name = StructClass->GetFName();
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

FDcResult FDcReadStateStruct::SkipRead()
{
	if (State != EState::ExpectValue)
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
		<< EState::ExpectValue << State
		<< _GetPropertyReader()->FormatHighlight();

	EndValueRead();
	return DcOk();
}

void FDcReadStateStruct::EndValueRead()
{
	check(State == EState::ExpectValue)
	Property = DcPropertyUtils::NextEffectiveProperty(Property);
	if (Property == nullptr)
		State = EState::ExpectEnd;
	else
		State = EState::ExpectKey;
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
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(MapProperty->KeyProp));
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(MapProperty->ValueProp));
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
	DC_TRY(ReadDataEntry(FNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastFieldChecked<FNameProperty>()->GetPropertyValue(Datum.DataPtr));
	return DcOk();
}

FDcResult FDcReadStateMap::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();

	FScriptMapHelper MapHelper(MapProperty, MapPtr);
	if (State == EState::ExpectKey)
	{
		FProperty* KeyProperty = MapHelper.GetKeyProperty();
		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(KeyProperty, ExpectedPropertyClass));

		OutDatum.Property = KeyProperty;
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FProperty* ValueProperty = MapHelper.GetValueProperty();
		DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(ValueProperty, ExpectedPropertyClass));

		OutDatum.Property = ValueProperty;
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);

		Index += 1;
		if (Index < MapHelper.Num())
			State = EState::ExpectKey;
		else
			State = EState::ExpectEnd;

		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

void FDcReadStateMap::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatMap(OutSegments, SegType, MapProperty, Index,
		State == EState::ExpectKey || State == EState::ExpectValue);
}

FDcResult FDcReadStateMap::ReadMapRoot()
{
	if (State == EState::ExpectRoot)
	{
		//	check map effectiveness
		if (!DcPropertyUtils::IsEffectiveProperty(MapProperty->KeyProp)
			|| !DcPropertyUtils::IsEffectiveProperty(MapProperty->ValueProp))
		{
			State = EState::ExpectEnd;
			return DcOk();
		}

		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		if (MapHelper.Num() == 0)
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

FDcResult FDcReadStateMap::SkipRead()
{
	if (State == EState::ExpectKey)
	{
		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		Index += 1;
		if (Index < MapHelper.Num())
			State = EState::ExpectKey;
		else
			State = EState::ExpectEnd;

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect2)
			<< EState::ExpectKey << EState::ExpectValue << State
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
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(ArrayProperty->Inner));
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
	DC_TRY(ReadDataEntry(FNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastFieldChecked<FNameProperty>()->GetPropertyValue(Datum.DataPtr));
	return DcOk();
}

FDcResult FDcReadStateArray::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
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

	Index += 1;
	if (Index < ArrayHelper.Num())
		State = EState::ExpectItem;
	else
		State = EState::ExpectEnd;

	return DcOk();
}

void FDcReadStateArray::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatArray(OutSegments, SegType, ArrayProperty, Index,
		State == EState::ExpectItem);
}

FDcResult FDcReadStateArray::ReadArrayRoot()
{
	if (State == EState::ExpectRoot)
	{
		if (!DcPropertyUtils::IsEffectiveProperty(ArrayProperty->Inner))
		{
			State = EState::ExpectEnd;
			return DcOk();
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
		if (ArrayHelper.Num() == 0)
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

FDcResult FDcReadStateArray::SkipRead()
{
	if (State == EState::ExpectItem)
	{
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);

		Index += 1;
		if (Index < ArrayHelper.Num())
			State = EState::ExpectItem;
		else
			State = EState::ExpectEnd;

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectItem << State
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
		ReadOut(OutPtr, DcPropertyUtils::PropertyToDataEntry(SetProperty->ElementProp));
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
	DC_TRY(ReadDataEntry(FNameProperty::StaticClass(), Datum));

	ReadOut(OutNamePtr, Datum.CastFieldChecked<FNameProperty>()->GetPropertyValue(Datum.DataPtr));

	return DcOk();
}

FDcResult FDcReadStateSet::ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State != EState::ExpectItem)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State << _GetPropertyReader()->FormatHighlight();

	check(State == EState::ExpectItem);
	FScriptSetHelper SetHelper(SetProperty, SetPtr);

	DC_TRY(DcPropertyReadStateDetails::CheckExpectedProperty(SetProperty->ElementProp, ExpectedPropertyClass));

	OutDatum.Property = SetProperty->ElementProp;
	OutDatum.DataPtr = SetHelper.GetElementPtr(Index);

	Index += 1;
	if (Index < SetHelper.Num())
		State = EState::ExpectItem;
	else
		State = EState::ExpectEnd;

	return DcOk();
}

void FDcReadStateSet::FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType)
{
	DcPropertyHighlight::FormatSet(OutSegments, SegType, SetProperty, Index,
		State == EState::ExpectItem);
}

FDcResult FDcReadStateSet::SkipRead()
{
	if (State == EState::ExpectItem)
	{
		FScriptSetHelper SetHelper(SetProperty, SetPtr);

		Index += 1;
		if (Index < SetHelper.Num())
			State = EState::ExpectItem;
		else
			State = EState::ExpectEnd;

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< EState::ExpectItem << State
			<< _GetPropertyReader()->FormatHighlight();
	}
}

FDcResult FDcReadStateSet::ReadSetRoot()
{
	if (State == EState::ExpectRoot)
	{
		if (!DcPropertyUtils::IsEffectiveProperty(SetProperty->ElementProp))
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
