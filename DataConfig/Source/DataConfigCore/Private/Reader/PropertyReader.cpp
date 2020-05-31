#include "Reader/PropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "DataConfigErrorCodes.h"
#include "PropretyCommon/PropertyReadStates.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

using ReaderStorageType = FPropertyReader::FPropertyState::ImplStorageType;

static FORCEINLINE ReaderStorageType* GetTopStorage(FPropertyReader* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FBaseState& GetTopState(FPropertyReader* Self)
{
	return *reinterpret_cast<FBaseState*>(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FPropertyReader* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FPropertyReader* Self) {
	return GetTopState(Self).As<TState>();
}

static void PushNilState(FPropertyReader* Reader) 
{
	Reader->States.AddDefaulted();
	Emplace<FStateNil>(GetTopStorage(Reader));
}

static void PushClassPropertyState(FPropertyReader* Reader, UObject* InClassObject)
{
	Reader->States.AddDefaulted();
	Emplace<FStateClass>(GetTopStorage(Reader), InClassObject);
}

static void PushStructPropertyState(FPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass)
{
	Reader->States.AddDefaulted();
	Emplace<FStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass);
}

static void PushMappingPropertyState(FPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	Emplace<FStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

static void PopState(FPropertyReader* Reader)
{
	Reader->States.Pop();
	check(Reader->States.Num() >= 1);
}

template<typename TState>
static void PopState(FPropertyReader* Reader)
{
	check(TState::ID == GetTopState(Reader).GetType());
	PopState(Reader);
}

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
	if (FStateStruct* StructState = TryGetTopState<FStateStruct>(this))
	{
		TRY(StructState->ReadStructRoot(OutNamePtr, CtxPtr));

		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

FResult FPropertyReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (FStateStruct* StructState = TryGetTopState<FStateStruct>(this))
	{
		TRY(StructState->ReadStructEnd(OutNamePtr, CtxPtr));

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
