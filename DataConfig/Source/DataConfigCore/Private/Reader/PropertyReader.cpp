#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "Reader/PropertyReader.h"
#include "DataConfigErrorCodes.h"
#include "PropretyCommon/PropertyStates.h"
#include "PropretyCommon/PropertyUtils.h"

namespace DataConfig {

static FORCEINLINE ReaderState& GetTopState(FPropertyReader* Self)
{
	return *reinterpret_cast<ReaderState*>(&Self->States.Top().ImplStorage);
}

template<typename TState>
static TState& GetTopState(FPropertyReader* Self) {
	return GetTopState(Self).Get<TState>();
}

template<typename TState>
static TState* TryGetTopState(FPropertyReader* Self) {
	return GetTopState(Self).TryGet<TState>();
}

static void PushNilState(FPropertyReader* Reader) 
{
	Reader->States.AddDefaulted();
}

static void PushClassRootState(FPropertyReader* Reader, UObject* ClassObject)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateClassRoot>(ClassObject);
}

static void PushStructRootState(FPropertyReader* Reader, void* StructPtr, UScriptStruct* StructClass)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateStructRoot>(StructPtr, StructClass);
}

static void PushClassPropertyState(FPropertyReader* Reader, UObject* ClassObject, UProperty* Property)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateClassProperty>(ClassObject, Property);
}

static void PushStructPropertyState(FPropertyReader* Reader, void* StructPtr, UScriptStruct* StructClass, UProperty* Property)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateStructProperty>(StructPtr, StructClass, Property);
}

static void PopState(FPropertyReader* Reader)
{
	Reader->States.Pop();
	check(Reader->States.Num() >= 1);
}

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode>
FResult TryGetPrimitive(FPropertyReader* Reader, TPrimitive& OutT)
{
	if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(ClassPropertyState->Property))
		{
			OutT = OutProperty->GetPropertyValue(
				OutProperty->ContainerPtrToValuePtr<TPrimitive>(ClassPropertyState->ClassObject)
			);
			//	TODO iterate
			return Ok();
		}
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(StructPropertyState->Property))
		{
			OutT = OutProperty->GetPropertyValue(
				OutProperty->ContainerPtrToValuePtr<TPrimitive>(StructPropertyState->StructPtr)
			);
			StructPropertyState->Property = NextEffectiveProperty(StructPropertyState->Property);
			return Ok();
		}
	}
	else if (StatePrimitive* PrimitiveState = TryGetTopState<StatePrimitive>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(PrimitiveState->Property))
		{
			OutT = OutProperty->GetPropertyValue(PrimitiveState->PrimitivePtr);
			return Ok();
		}
	}

	return Fail(ErrCode);
}

template<typename TPrimitive, typename TProperty, EErrorCode ErrCode>
FResult ReadPrimitive(FPropertyReader* Reader, TPrimitive* OutPtr)
{
	return TryGetPrimitive<TPrimitive, TProperty, ErrCode>(Reader, *OutPtr);
}


FPropertyReader::FPropertyReader()
{
	PushNilState(this);
}

FPropertyReader::~FPropertyReader()
{}

EDataEntry FPropertyReader::Peek()
{
	if (StateNil* NilStatePtr = TryGetTopState<StateNil>(this))
	{
		return EDataEntry::Ended;
	}
	else if (StateClassRoot* ClassRootState = TryGetTopState<StateClassRoot>(this))
	{
		return EDataEntry::ClassRoot;
	}
	else if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		return EDataEntry::StructRoot;
	}
	else if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(this))
	{
		return PropertyToDataEntry(ClassPropertyState->Property);
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		return PropertyToDataEntry(StructPropertyState->Property);
	}

	return EDataEntry::Ended;
}

FResult FPropertyReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<bool, UBoolProperty, EErrorCode::ExpectBoolFail>(this, *OutPtr);
}

FResult FPropertyReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, *OutPtr);
}

FResult FPropertyReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<FString, UStrProperty, EErrorCode::ExpectStringFail>(this, *OutPtr);
}

FResult FPropertyReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		if (OutNamePtr != nullptr)
		{
			*OutNamePtr = StructRootState->StructClass->GetFName();
		}

		PushStructPropertyState(this,
			StructRootState->StructPtr,
			StructRootState->StructClass,
			FirstEffectiveProperty(StructRootState->StructClass->PropertyLink)
		);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ExpectStructFail);
	}
}

FResult FPropertyReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (NextEffectiveProperty(StructPropertyState->Property) != nullptr)
		{
			return Fail(EErrorCode::StructEndWhenStillHasValue);
		}

		if (OutNamePtr != nullptr)
		{
			*OutNamePtr = StructPropertyState->StructClass->GetFName();
		}

		PopState(this);
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::ExpectStructEndFail);
	}
}

FPropertyReader::FPropertyState::FPropertyState()
{
	static_assert(sizeof(FPropertyState) <= 64, "larger than cacheline");
	static_assert(sizeof(ReaderState) <= sizeof(FPropertyState), "impl storage size too small");
	//	TODO drop the TVariant and use a pod type, as we know it's a pod
	new(&ImplStorage) ReaderState(TInPlaceType<StateNil>{});
}



} // namespace DataConfig
