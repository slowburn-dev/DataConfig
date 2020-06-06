#include "Reader/PropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "DataConfigErrorCodes.h"
#include "PropretyCommon/PropertyReadStates.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

FPropertyReader::FPropertyReader()
{
	PushNilState(this);
}

FPropertyReader::FPropertyReader(FPropertyDatum Datum)
	: FPropertyReader()
{
	if (Datum.IsNone())
	{
		//	pass
	}
	else if (Datum.Property->IsA<UClass>()) 
	{
		UObject* Obj = reinterpret_cast<UObject*>(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassPropertyState(this, Obj);
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructPropertyState(this, Datum.DataPtr, CastChecked<UScriptStruct>(Datum.Property));
	}
	else
	{
		checkNoEntry();
	}
}

EDataEntry FPropertyReader::Peek()
{
	return GetTopState(this).Peek();
}

FResult FPropertyReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(GetTopState(this).ReadDataEntry(UBoolProperty::StaticClass(), EErrorCode::ReadBoolFail, CtxPtr, Datum));

	if (OutPtr)
	{
		*OutPtr = Datum.As<UBoolProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FPropertyReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	TRY(GetTopState(this).ReadName(OutPtr, CtxPtr));

	return Ok();
}

FResult FPropertyReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(GetTopState(this).ReadDataEntry(UStrProperty::StaticClass(), EErrorCode::ReadStringFail, CtxPtr, Datum));

	if (OutPtr)
	{
		*OutPtr = Datum.As<UStrProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FPropertyReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FBaseState& TopState = GetTopState(this);
	{
		FStateStruct* StructState = TopState.As<FStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FStateStruct::EState::ExpectRoot)
		{
			TRY(StructState->ReadStructRoot(OutNamePtr, CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UStructProperty::StaticClass(), EErrorCode::ReadStructFail, CtxPtr, Datum));

		FStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.As<UStructProperty>()->Struct);
		TRY(ChildStruct.ReadStructRoot(OutNamePtr, CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (FStateStruct* StructState = TryGetTopState<FStateStruct>(this))
	{
		TRY(StructState->ReadStructEnd(OutNamePtr, CtxPtr));
		PopState<FStateStruct>(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

DataConfig::FResult FPropertyReader::ReadClassRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FBaseState& TopState = GetTopState(this);
	{
		FStateClass* ClassState = TopState.As<FStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FStateClass::EState::ExpectRoot)
		{
			TRY(ClassState->ReadClassRoot(OutNamePtr, CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UClassProperty::StaticClass(), EErrorCode::ReadClassFail, CtxPtr, Datum));

		FStateClass& ChildClass = PushClassPropertyState(this, (UObject*)Datum.DataPtr);
		TRY(ChildClass.ReadClassEnd(OutNamePtr, CtxPtr));
	}

	return Ok();
}

DataConfig::FResult FPropertyReader::ReadClassEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (FStateClass* ClassState = TryGetTopState<FStateClass>(this))
	{
		TRY(ClassState->ReadClassEnd(OutNamePtr, CtxPtr));
		PopState<FStateClass>(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadClassEndFail);
	}
}

FResult FPropertyReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	FBaseState& TopState = GetTopState(this);
	{
		FStateMap* MapState = TopState.As<FStateMap>();
		if (MapState != nullptr
			&& MapState->State == FStateMap::EState::ExpectRoot)
		{
			TRY(MapState->ReadMapRoot(CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UMapProperty::StaticClass(), EErrorCode::ReadMapFail, CtxPtr, Datum));

		FStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.As<UMapProperty>());
		TRY(ChildMap.ReadMapRoot(CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (FStateMap* StateMap = TryGetTopState<FStateMap>(this))
	{
		TRY(StateMap->ReadMapEnd(CtxPtr));
		PopState<FStateMap>(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ReadMapFail);
	}
}

FResult FPropertyReader::ReadArrayRoot(FContextStorage* CtxPtr)
{
	FBaseState& TopState = GetTopState(this);

	{
		FStateArray* ArrayState = TopState.As<FStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FStateArray::EState::ExpectRoot)
		{
			TRY(ArrayState->ReadArrayRoot(CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UArrayProperty::StaticClass(), EErrorCode::ReadArrayFail, CtxPtr, Datum));

		FStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.As<UArrayProperty>());
		TRY(ChildArray.ReadArrayRoot(CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadArrayEnd(FContextStorage* CtxPtr)
{
	if (FStateArray* ArrayState = TryGetTopState<FStateArray>(this))
	{
		TRY(ArrayState->ReadArrayEnd(CtxPtr));
		PopState<FStateArray>(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

} // namespace DataConfig
