#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

FDcResult FDcBaseReadState::PeekRead(EDcDataEntry* OutPtr)
{
	return DC_FAIL(DcDCommon, NotImplemented);
}

FDcResult FDcBaseReadState::ReadName(FName* OutNamePtr)
{
	return DC_FAIL(DcDCommon, NotImplemented);
}

FDcResult FDcBaseReadState::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	return DC_FAIL(DcDCommon, NotImplemented);
}

EDcPropertyReadType FDcReadStateNil::GetType()
{
	return EDcPropertyReadType::Nil;
}

FDcResult FDcReadStateNil::PeekRead(EDcDataEntry* OutPtr)
{
	*OutPtr = EDcDataEntry::Ended;
	return DcOk();
}

EDcPropertyReadType FDcReadStateClass::GetType()
{
	return EDcPropertyReadType::ClassProperty;
}

FDcResult FDcReadStateClass::PeekRead(EDcDataEntry* OutPtr)
{
	if (State == EState::ExpectRoot)
	{
		*OutPtr = EDcDataEntry::ClassRoot;
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		*OutPtr = EDcDataEntry::ClassEnd;
		return DcOk();
	}
	else if (State == EState::ExpectNil)
	{
		*OutPtr = EDcDataEntry::Nil;
		return DcOk();
	}
	else if (State == EState::ExpectReference)
	{
		*OutPtr = EDcDataEntry::Reference;
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		*OutPtr = EDcDataEntry::Name;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		*OutPtr = PropertyToDataEntry(Property);
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcReadStateClass::ReadName(FName* OutNamePtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FDcPropertyDatum Datum;
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcReadStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// should've be already handled in `ReadName`
		return DC_FAIL(DcDCommon, Unreachable);
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);
		if (Property->IsA(ExpectedPropertyClass))
		{
			OutDatum.Property = Property;
			OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(ClassObject);

			EndReadValue();
			return DcOk();
		}
		else
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->GetFName();
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
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
			<< (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FDcReadStateClass::ReadClassRoot(FDcClassPropertyStat* OutClassPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassObject == nullptr)
		{
			check(Type == EType::PropertyNormal || Type == EType::PropertyInstanced);
			if (OutClassPtr)
			{
				OutClassPtr->Name = Class->GetFName();
				OutClassPtr->Reference = EDcDataReference::NullReference;
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
					OutClassPtr->Reference = EDcDataReference::ExternalReference;
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
					OutClassPtr->Reference = EDcDataReference::ExpandObject;
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
				checkNoEntry();
				return DC_FAIL(DcDCommon, Unreachable);
			}
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FDcReadStateClass::ReadClassEnd(FDcClassPropertyStat* OutClassPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutClassPtr)
		{
			OutClassPtr->Name = Class->GetFName();
			OutClassPtr->Reference = ClassObject == nullptr
				? EDcDataReference::NullReference
				: EDcDataReference::ExpandObject;
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State;
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
			<< (int)EState::ExpectNil << (int)State;
	}
}

FDcResult FDcReadStateClass::ReadReference(UObject** OutPtr)
{
	if (State == EState::ExpectReference)
	{
		if (OutPtr)
		{
			*OutPtr = ClassObject;
		}

		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State;
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
		*OutPtr = EDcDataEntry::StructRoot;
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		*OutPtr = EDcDataEntry::StructEnd;
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		*OutPtr = EDcDataEntry::Name;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		*OutPtr = PropertyToDataEntry(Property);
		return DcOk();
	}
	else if (State == EState::Ended)
	{
		*OutPtr = EDcDataEntry::Ended;
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcReadStateStruct::ReadName(FName* OutNamePtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		FDcPropertyDatum Datum;
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;
	}
}

FDcResult FDcReadStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// shouldn't be hanndled in `ReadName`
		return DC_FAIL(DcDCommon, Unreachable);
	}
	else if (State == EState::ExpectValue)
	{
		check(ExpectedPropertyClass);
		check(Property);
		if (Property->IsA(ExpectedPropertyClass))
		{
			OutDatum.Property = Property;
			OutDatum.DataPtr = Property->ContainerPtrToValuePtr<void>(StructPtr);

			EndReadValue();	// this now guarentee success
			return DcOk();
		}
		else
		{
			return DC_FAIL(DcDReadWrite, PropertyMismatch)
				<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->GetFName();
		}
	}
	else
	{
		return DC_FAIL(DcDReadWrite, InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
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
			<< (int)EState::ExpectValue << (int)State;
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
			<< (int)EState::ExpectRoot << (int)State;
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
			<< (int)EState::ExpectEnd << (int)State;
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
		*OutPtr = EDcDataEntry::MapRoot;
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		*OutPtr = EDcDataEntry::MapEnd;
		return DcOk();
	}
	else if (State == EState::ExpectKey)
	{
		check(MapProperty);
		*OutPtr = PropertyToDataEntry(MapProperty->KeyProp);
		return DcOk();
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		*OutPtr = PropertyToDataEntry(MapProperty->ValueProp);
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcReadStateMap::ReadName(FName* OutNamePtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FDcReadStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;

	FScriptMapHelper MapHelper(MapProperty, MapPtr);
	if (State == EState::ExpectKey)
	{
		OutDatum.Property = MapHelper.GetKeyProperty();
		OutDatum.DataPtr = MapHelper.GetKeyPtr(Index);
	}
	else if (State == EState::ExpectValue)
	{
		OutDatum.Property = MapHelper.GetValueProperty();
		OutDatum.DataPtr = MapHelper.GetValuePtr(Index);
	}
	else
	{
		checkNoEntry();
	}

	DC_TRY(EndReadValue());
	return DcOk();
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
			<< (int)State;
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
			<< (int)EState::ExpectRoot << (int)State;
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
			<< (int)EState::ExpectEnd << (int)State;
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
		*OutPtr = EDcDataEntry::ArrayRoot;
		return DcOk();
	}
	else if (State == EState::ExpectEnd)
	{
		*OutPtr = EDcDataEntry::ArrayEnd;
		return DcOk();
	}
	else if (State == EState::ExpectItem)
	{
		check(ArrayProperty->Inner);
		*OutPtr = PropertyToDataEntry(ArrayProperty->Inner);
		return DcOk();
	}
	else if (State == EState::Ended)
	{
		*OutPtr = EDcDataEntry::Ended;
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcReadStateArray::ReadName(FName* OutNamePtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FDcReadStateArray::ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DC_FAIL(DcDReadWrite, InvalidStateNoExpect)
			<< (int)State;

	check(State == EState::ExpectItem);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	DC_TRY(EndReadValue());
	return DcOk();
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
			<< (int)EState::ExpectItem << (int)State;
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
			<< (int)EState::ExpectRoot << (int)State;
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
			<< (int)EState::ExpectEnd << (int)State;
	}
}

