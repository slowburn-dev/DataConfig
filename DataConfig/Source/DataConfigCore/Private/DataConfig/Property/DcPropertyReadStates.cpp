#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticPropertyReadWrite.h"

namespace DataConfig {

EDataEntry FBaseReadState::Peek()
{
	return EDataEntry::Ended;
}

FResult FBaseReadState::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	return Fail(DCommon::Category, DCommon::NotImplemented);
}

FResult FBaseReadState::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	return Fail(DCommon::Category, DCommon::NotImplemented);
}

EPropertyReadType FReadStateNil::GetType()
{
	return EPropertyReadType::Nil;
}

EPropertyReadType FReadStateClass::GetType()
{
	return EPropertyReadType::ClassProperty;
}

EDataEntry FReadStateClass::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::ClassRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::ClassEnd;
	}
	else if (State == EState::ExpectNil)
	{
		return EDataEntry::Nil;
	}
	else if (State == EState::ExpectReference)
	{
		return EDataEntry::Reference;
	}
	else if (State == EState::ExpectKey)
	{
		return EDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateClass::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateNoExpect)
			<< (int)State;
	}
}

FResult FReadStateClass::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// should've be already handled in `ReadName`
		return Fail(DCommon::Category, DCommon::Unreachable);
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
			return Ok();
		}
		else
		{
			return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::PropertyMismatch)
				<< ExpectedPropertyClass->ClassConfigName << Property->GetName() << Property->GetClass()->ClassConfigName;
		}
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
}

FResult FReadStateClass::EndReadValue()
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
}

FResult FReadStateClass::ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (ClassObject == nullptr)
		{
			check(Type == EType::PropertyNormal || Type == EType::PropertyInstanced);
			if (OutClassPtr)
			{
				OutClassPtr->Name = Class->GetFName();
				OutClassPtr->Reference = EDataReference::NullReference;
			}

			State = EState::ExpectNil;
			return Ok();
		}
		else
		{
			if (Type == EType::PropertyNormal)
			{
				State = EState::ExpectReference;

				if (OutClassPtr)
				{
					OutClassPtr->Name = Class->GetFName();
					OutClassPtr->Reference = EDataReference::ExternalReference;
				}

				return Ok();
			}
			else if (Type == EType::PropertyInstanced
				|| Type == EType::Root)
			{
				check(ClassObject);
				UClass* Cls = ClassObject->GetClass();
				if (OutClassPtr)
				{
					OutClassPtr->Name = Cls->GetFName();
					OutClassPtr->Reference = EDataReference::ExpandObject;
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
				return Ok();
			}
			else
			{
				checkNoEntry();
				return Fail(DCommon::Category, DCommon::Unreachable);
			}
		}
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FReadStateClass::ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;

		if (OutClassPtr)
		{
			OutClassPtr->Name = Class->GetFName();
			OutClassPtr->Reference = ClassObject == nullptr
				? EDataReference::NullReference
				: EDataReference::ExpandObject;
		}

		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State;
	}
}

FResult FReadStateClass::ReadNil(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectNil)
	{
		State = EState::ExpectEnd;
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectNil << (int)State;
	}
}

FResult FReadStateClass::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectReference)
	{
		if (OutPtr)
		{
			*OutPtr = ClassObject;
		}

		State = EState::ExpectEnd;
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectReference << (int)State;
	}
}

EPropertyReadType FReadStateStruct::GetType()
{
	return EPropertyReadType::StructProperty;
}

EDataEntry FReadStateStruct::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::StructRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::StructEnd;
	}
	else if (State == EState::ExpectKey)
	{
		return EDataEntry::Name;
	}
	else if (State == EState::ExpectValue)
	{
		check(Property);
		return PropertyToDataEntry(Property);
	}
	else if (State == EState::Ended)
	{
		return EDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateStruct::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (State == EState::ExpectKey)
	{
		check(Property);
		if (OutNamePtr)
		{
			*OutNamePtr = Property->GetFName();
		}

		State = EState::ExpectValue;
		return Ok();
	}
	else if (State == EState::ExpectValue)
	{
		FPropertyDatum Datum;
		TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

		if (OutNamePtr)
		{
			*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
		}

		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateNoExpect)
			<< (int)State;
	}
}

FResult FReadStateStruct::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::ExpectKey)
	{
		checkNoEntry();	// shouldn't be hanndled in `ReadName`
		return Fail(DCommon::Category, DCommon::Unreachable);
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
			return Ok();
		}
		else
		{
			return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::PropertyMismatch)
				<< ExpectedPropertyClass->ClassConfigName << Property->GetName() << Property->GetClass()->ClassConfigName;
		}
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
}

FResult FReadStateStruct::EndReadValue()
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectValue << (int)State;
	}
}

FResult FReadStateStruct::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
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

		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FReadStateStruct::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State;
	}
}

EPropertyReadType FReadStateMap::GetType()
{
	return EPropertyReadType::MapProperty;
}

EDataEntry FReadStateMap::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::MapRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::MapEnd;
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
		return EDataEntry::Ended;
	}
}

FResult FReadStateMap::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FReadStateMap::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return Fail(FailCode);

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

	TRY(EndReadValue());
	return Ok();
}

FResult FReadStateMap::EndReadValue()
{
	if (State == EState::ExpectKey)
	{
		State = EState::ExpectValue;
		return Ok();
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateNoExpect)
			<< (int)State;
	}
}

FResult FReadStateMap::ReadMapRoot(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		//	check map effectiveness
		if (!IsEffectiveProperty(MapProperty->KeyProp)
			|| !IsEffectiveProperty(MapProperty->ValueProp))
		{
			State = EState::ExpectEnd;
			return Ok();
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FReadStateMap::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State;
	}
}

EPropertyReadType FReadStateArray::GetType()
{
	return EPropertyReadType::ArrayProperty;
}

EDataEntry FReadStateArray::Peek()
{
	if (State == EState::ExpectRoot)
	{
		return EDataEntry::ArrayRoot;
	}
	else if (State == EState::ExpectEnd)
	{
		return EDataEntry::ArrayEnd;
	}
	else if (State == EState::ExpectItem)
	{
		check(ArrayProperty->Inner);
		return PropertyToDataEntry(ArrayProperty->Inner);
	}
	else if (State == EState::Ended)
	{
		return EDataEntry::Ended;
	}
	else
	{
		checkNoEntry();
		return EDataEntry::Ended;
	}
}

FResult FReadStateArray::ReadName(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(ReadDataEntry(UNameProperty::StaticClass(), EErrorCode::ReadNameFail, CtxPtr, Datum));

	if (OutNamePtr)
	{
		*OutNamePtr = Datum.CastChecked<UNameProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FReadStateArray::ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum)
{
	if (State == EState::Ended
		|| State == EState::ExpectRoot
		|| State == EState::ExpectEnd)
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateNoExpect)
			<< (int)State;

	check(State == EState::ExpectItem);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
	OutDatum.Property = ArrayProperty->Inner;
	OutDatum.DataPtr = ArrayHelper.GetRawPtr(Index);

	TRY(EndReadValue());
	return Ok();
}

FResult FReadStateArray::EndReadValue()
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectItem << (int)State;
	}
}

FResult FReadStateArray::ReadArrayRoot(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectRoot)
	{
		if (!IsEffectiveProperty(ArrayProperty->Inner))
		{
			State = EState::ExpectEnd;
			return Ok();
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
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectRoot << (int)State;
	}
}

FResult FReadStateArray::ReadArrayEnd(FContextStorage* CtxPtr)
{
	if (State == EState::ExpectEnd)
	{
		State = EState::Ended;
		return Ok();
	}
	else
	{
		return Fail(DPropertyReadWrite::Category, DPropertyReadWrite::InvalidStateWithExpect)
			<< (int)EState::ExpectEnd << (int)State;
	}
}

} // namespace DataConfig


