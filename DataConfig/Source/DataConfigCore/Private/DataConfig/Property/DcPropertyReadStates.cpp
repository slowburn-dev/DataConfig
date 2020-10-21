#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

EDcDataEntry FBaseReadState::Peek()
{
	return EDcDataEntry::Ended;
}

FDcResult FBaseReadState::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return DcFail(DC_DIAG(DcDCommon, NotImplemented));
}

FDcResult FBaseReadState::ReadDataEntry(UClass* ExpectedPropertyClass, FContextStorage* CtxPtr, FDcPropertyDatum& OutDatum)
{
	return DcFail(DC_DIAG(DcDCommon, NotImplemented));
}

EPropertyReadType FReadStateNil::GetType()
{
	return EPropertyReadType::Nil;
}

EPropertyReadType FReadStateClass::GetType()
{
	return EPropertyReadType::ClassProperty;
}

EDcDataEntry FReadStateClass::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDcDataEntry::ClassRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDcDataEntry::ClassEnd;
	}
	else if (State == EState::ExpectNil)
	{
		return EDcDataEntry::Nil;
	}
	else if (State == EState::ExpectReference)
	{
		return EDcDataEntry::Reference;
	}
	else if (State == EState::ExpectKey)
	{
		return EDcDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else
	{
		checkNoEntry();
		return EDcDataEntry::Ended;
	}
}

FDcResult FReadStateClass::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
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
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FReadStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, FContextStorage* CtxPtr, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// should've be already handled in `ReadName`
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
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
			return DcFail(DC_DIAG(DcDReadWrite, PropertyMismatch))
				<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->GetFName();
		}
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FReadStateClass::EndReadValue()
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FReadStateClass::ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
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
				return DcFail(DC_DIAG(DcDCommon, Unreachable));
			}
		}
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FReadStateClass::ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectEnd << (int)State;
	}
}

FDcResult FReadStateClass::ReadNil(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectNil)
	{
		State = EState::ExpectEnd;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectNil << (int)State;
	}
}

FDcResult FReadStateClass::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectReference << (int)State;
	}
}

EPropertyReadType FReadStateStruct::GetType()
{
	return EPropertyReadType::StructProperty;
}

EDcDataEntry FReadStateStruct::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDcDataEntry::StructRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDcDataEntry::StructEnd;
	}
	else if (State == EState::ExpectKey)
	{
		return EDcDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else if (State == EState::Ended)
	{
		return EDcDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDcDataEntry::Ended;
	}
}

FDcResult FReadStateStruct::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
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
		DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FReadStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, FContextStorage* CtxPtr, FDcPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// shouldn't be hanndled in `ReadName`
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
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
			return DcFail(DC_DIAG(DcDReadWrite, PropertyMismatch))
				<< ExpectedPropertyClass->ClassConfigName << Property->GetFName() << Property->GetClass()->GetFName();
		}
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FReadStateStruct::EndReadValue()
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectValue << (int)State;
	}
}

FDcResult FReadStateStruct::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FReadStateStruct::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectEnd << (int)State;
	}
}

EPropertyReadType FReadStateMap::GetType()
{
	return EPropertyReadType::MapProperty;
}

EDcDataEntry FReadStateMap::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDcDataEntry::MapRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDcDataEntry::MapEnd;
	}
	else if (State == EState::ExpectKey)
	{
		check(MapProperty);
		return PropertyToDataEntry(MapProperty->KeyProp);
	}
	else if (State == EState::ExpectValue)
	{
		check(MapProperty);
		return PropertyToDataEntry(MapProperty->ValueProp);
	}
	else
	{
		checkNoEntry();
		return EDcDataEntry::Ended;
	}
}

FDcResult FReadStateMap::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FReadStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, FContextStorage* CtxPtr, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
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

FDcResult FReadStateMap::EndReadValue()
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;
	}
}

FDcResult FReadStateMap::ReadMapRoot(FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FReadStateMap::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectEnd << (int)State;
	}
}

EPropertyReadType FReadStateArray::GetType()
{
	return EPropertyReadType::ArrayProperty;
}

EDcDataEntry FReadStateArray::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDcDataEntry::ArrayRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDcDataEntry::ArrayEnd;
	}
	else if (State == EState::ExpectItem)
	{
		check(ArrayProperty->Inner);
		return PropertyToDataEntry(ArrayProperty->Inner);
	}
	else if (State == EState::Ended)
	{
		return EDcDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDcDataEntry::Ended;
	}
}

FDcResult FReadStateArray::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FDcPropertyDatum Datum;
	DC_TRY(ReadDataEntry(UNameProperty::StaticClass(), CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return DcOk();
}

FDcResult FReadStateArray::ReadDataEntry(UClass* ExpectedPropertyClass, FContextStorage* CtxPtr, FDcPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateNoExpect))
			<< (int)State;

	check(State == EState::ExpectItem);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	DC_TRY(EndReadValue());
	return DcOk();
}

FDcResult FReadStateArray::EndReadValue()
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectItem << (int)State;
	}
}

FDcResult FReadStateArray::ReadArrayRoot(FContextStorage* CtxPtr)
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
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FDcResult FReadStateArray::ReadArrayEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDReadWrite, InvalidStateWithExpect))
			<< (int)EState::ExpectEnd << (int)State;
	}
}

