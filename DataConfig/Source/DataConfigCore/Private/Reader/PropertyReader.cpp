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
FResult TryGetPrimitive(FPropertyReader* Reader, TPrimitive* OutPtr)
{
	if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(ClassPropertyState->Property))
		{
			if (OutPtr != nullptr)
			{
				*OutPtr = OutProperty->GetPropertyValue(
					OutProperty->ContainerPtrToValuePtr<TPrimitive>(ClassPropertyState->ClassObject)
				);
			}
			//	TODO iterate
			return Ok();
		}
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(StructPropertyState->Property))
		{
			if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
			{
				//	handled in `ReadName`
				return Fail(EErrorCode::StructExpectKeyFail);	
			}
			else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
			{
				if (OutPtr != nullptr)
				{
					*OutPtr = OutProperty->GetPropertyValue(
						OutProperty->ContainerPtrToValuePtr<TPrimitive>(StructPropertyState->StructPtr)
					);
				}

				StructPropertyState->Property = NextEffectiveProperty(StructPropertyState->Property);
				if (StructPropertyState->Property != nullptr)
				{
					StructPropertyState->State = StateStructProperty::EState::ExpectKey;
				}
				else
				{
					StructPropertyState->State = StateStructProperty::EState::Ended;
				}
				return Ok();
			}
			else if (StructPropertyState->State == StateStructProperty::EState::Ended)
			{
				return Fail(EErrorCode::StructReadAfterEnded);
			}
		}
	}
	else if (StatePrimitive* PrimitiveState = TryGetTopState<StatePrimitive>(Reader))
	{
		if (TProperty* OutProperty = Cast<TProperty>(PrimitiveState->Property))
		{
			if (OutPtr != nullptr)
			{
				*OutPtr = OutProperty->GetPropertyValue(PrimitiveState->PrimitivePtr);
			}
			return Ok();
		}
	}

	return Fail(ErrCode);
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
		PushClassRootState(this, Obj);
	}
	else if (Datum.Property->IsA<UScriptStruct>())
	{
		PushStructRootState(this,
			Datum.DataPtr,
			CastChecked<UScriptStruct>(Datum.Property)
		);
	}
	else
	{
		checkNoEntry();
	}
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
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			return EDataEntry::Name;
		}
		else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			return PropertyToDataEntry(StructPropertyState->Property);
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return EDataEntry::StructEnd;
		}
		checkNoEntry();
	}

	return EDataEntry::Ended;
}

FResult FPropertyReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<bool, UBoolProperty, EErrorCode::ExpectBoolFail>(this, OutPtr);
}

FResult FPropertyReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(this))
	{
		//	TODO
		return TryGetPrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, OutPtr);
	}
	else if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			if (OutPtr)
			{
				*OutPtr = StructPropertyState->Property->GetFName();
			}

			StructPropertyState->State = StateStructProperty::EState::ExpectValue;
			return Ok();
		}
		else
		{
			return TryGetPrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, OutPtr);
		}
	}
	else
	{
		return TryGetPrimitive<FName, UNameProperty, EErrorCode::ExpectNameFail>(this, OutPtr);
	}
}

FResult FPropertyReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<FString, UStrProperty, EErrorCode::ExpectStringFail>(this, OutPtr);
}

static void PushFirstStructPropertyState(FPropertyReader* Reader, FName* OutNamePtr, void* StructPtr, UScriptStruct* StructClass)
{
	if (OutNamePtr != nullptr)
	{
		*OutNamePtr = StructClass->GetFName();
	}

	PushStructPropertyState(Reader,
		StructPtr,
		StructClass,
		FirstEffectiveProperty(StructClass->PropertyLink)
	);
}

static FResult FinishTopStateValueRead(FPropertyReader* Reader)
{
	if (StateNil* NilState = TryGetTopState<StateNil>(Reader))
	{
		return Ok();
	}
	else if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(Reader))
	{
		//	TODO
		return Ok();
	}
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(Reader))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			StructPropertyState->Property = NextEffectiveProperty(StructPropertyState->Property);
			if (StructPropertyState->Property != nullptr)
			{
				StructPropertyState->State = StateStructProperty::EState::ExpectKey;
			}
			else
			{
				StructPropertyState->State = StateStructProperty::EState::Ended;
			}
			return Ok();
		}
		else
		{
			return Fail(EErrorCode::UnknownError);
		}
	}

	return Fail(EErrorCode::UnknownError);
}

FResult FPropertyReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			return Fail(EErrorCode::StructExpectKeyFail);	
		}
		else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			if (UStructProperty* StructProperty = Cast<UStructProperty>(StructPropertyState->Property))
			{
				//	note that this actually maps to the double Pop()
				PushStructRootState(this,
					StructProperty->ContainerPtrToValuePtr<void>(StructPropertyState->StructPtr),
					StructProperty->Struct
				);
				StateStructRoot& StructRootRef = GetTopState<StateStructRoot>(this);
				PushFirstStructPropertyState(this, OutNamePtr, StructRootRef.StructPtr, StructRootRef.StructClass);
				return Ok();
			}
			else
			{
				return Fail(EErrorCode::ExpectStructFail);
			}
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return Fail(EErrorCode::StructReadAfterEnded);
		}
		checkNoEntry();
	}
	else if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		PushFirstStructPropertyState(this, OutNamePtr, StructRootState->StructPtr, StructRootState->StructClass);
		return Ok();
	}

	return Fail(EErrorCode::ExpectStructFail);
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

		PopState(this);		// pops struct property
		PopState(this);		// pops struct root
		return FinishTopStateValueRead(this);
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
