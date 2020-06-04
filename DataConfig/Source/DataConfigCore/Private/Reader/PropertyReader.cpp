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
	else if (Datum.Property->IsA<UClassProperty>()) 
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

FResult FPropertyReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	return Ok();
}

FResult FPropertyReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	return Ok();
}

} // namespace DataConfig
