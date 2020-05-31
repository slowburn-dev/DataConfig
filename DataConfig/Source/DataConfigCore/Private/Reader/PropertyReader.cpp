#include "Reader/PropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
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

static void PushMappingRootState(FPropertyReader* Reader, void* MapPtr, UMapProperty* MapProperty)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateMappingRoot>(MapPtr, MapProperty);
}

static void PushMappingPropertyState(FPropertyReader* Reader, void* MapPtr, UMapProperty* MapProperty)
{
	Reader->States.AddDefaulted();
	GetTopState(Reader).Emplace<StateMappingProperty>(MapPtr, MapProperty);
}

//	TODO tempalte this with a assertion
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
				return Fail(EErrorCode::ReadStructKeyFail);	
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
				return Fail(EErrorCode::ReadStructAfterEnded);
			}
		}
	}
	else if (StateMappingProperty* MappingPropertyState = TryGetTopState<StateMappingProperty>(Reader))
	{
		UProperty* Property = nullptr;
		void* Ptr = nullptr;

		if (MappingPropertyState->State == StateMappingProperty::EState::ExpectKey)
		{
			Property = MappingPropertyState->MapHelper.GetKeyProperty();
			Ptr = MappingPropertyState->MapHelper.GetKeyPtr(MappingPropertyState->Index);
		}
		else if (MappingPropertyState->State == StateMappingProperty::EState::ExpectValue)
		{
			Property = MappingPropertyState->MapHelper.GetValueProperty();
			Ptr = MappingPropertyState->MapHelper.GetValuePtr(MappingPropertyState->Index);
		}
		else if (MappingPropertyState->State == StateMappingProperty::EState::Ended)
		{
			return Fail(EErrorCode::ReadMapAfterEnd);
		}
		else
		{
			checkNoEntry();
		}

		if (TProperty* OutProperty = Cast<TProperty>(Property))
		{
			if (OutPtr != nullptr)
			{
				*OutPtr = OutProperty->GetPropertyValue(Ptr);
			}

			if (MappingPropertyState->State == StateMappingProperty::EState::ExpectKey)
			{
				MappingPropertyState->State = StateMappingProperty::EState::ExpectValue;
			}
			else if (MappingPropertyState->State == StateMappingProperty::EState::ExpectValue)
			{
				MappingPropertyState->Index += 1;
				if (MappingPropertyState->Index >= MappingPropertyState->MapHelper.Num())
				{
					MappingPropertyState->State = StateMappingProperty::EState::Ended;
				}
				else
				{
					MappingPropertyState->State = StateMappingProperty::EState::ExpectKey;
				}
			}
			else
			{
				checkNoEntry();
			}

			return Ok();
		}

		return Fail(ErrCode);
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
	else if (StateMappingRoot* MappingRootState = TryGetTopState<StateMappingRoot>(this))
	{
		return EDataEntry::MapRoot;
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
	else if (StateMappingProperty* MapPropertyState = TryGetTopState<StateMappingProperty>(this))
	{
		if (MapPropertyState->State == StateMappingProperty::EState::ExpectKey)
		{
			return PropertyToDataEntry(MapPropertyState->MapHelper.GetKeyProperty());
		}
		else if (MapPropertyState->State == StateMappingProperty::EState::ExpectValue)
		{
			return PropertyToDataEntry(MapPropertyState->MapHelper.GetValueProperty());
		}
		else
		{
			return EDataEntry::MapEnd;
		}
		checkNoEntry();
	}

	return EDataEntry::Ended;
}

FResult FPropertyReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<bool, UBoolProperty, EErrorCode::ReadBoolFail>(this, OutPtr);
}

FResult FPropertyReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	if (StateClassProperty* ClassPropertyState = TryGetTopState<StateClassProperty>(this))
	{
		//	TODO
		return TryGetPrimitive<FName, UNameProperty, EErrorCode::ReadNameFail>(this, OutPtr);
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
			return TryGetPrimitive<FName, UNameProperty, EErrorCode::ReadNameFail>(this, OutPtr);
		}
	}
	else
	{
		return TryGetPrimitive<FName, UNameProperty, EErrorCode::ReadNameFail>(this, OutPtr);
	}
}

FResult FPropertyReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	return TryGetPrimitive<FString, UStrProperty, EErrorCode::ReadStringFail>(this, OutPtr);
}

static void PushFirstStructPropertyState(FPropertyReader* Reader, FName* OutNamePtr, void* StructPtr, UScriptStruct* StructClass)
{
	if (OutNamePtr != nullptr)
	{
		*OutNamePtr = StructClass->GetFName();
	}

	//	TODO 
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
	else if (StateMappingProperty* MappingPropertyState = TryGetTopState<StateMappingProperty>(Reader))
	{
		if (MappingPropertyState->State == StateMappingProperty::EState::ExpectKey)
		{
			//	MapOfStruct2
			MappingPropertyState->State = StateMappingProperty::EState::ExpectValue;
			return Ok();
		}
		else if (MappingPropertyState->State == StateMappingProperty::EState::ExpectValue)
		{
			//	actually this duplicates TryGetPrimitive()...
			//	so it's actually a bit weird
			MappingPropertyState->Index += 1;
			if (MappingPropertyState->Index >= MappingPropertyState->MapHelper.Num())
			{
				MappingPropertyState->State = StateMappingProperty::EState::Ended;
			}
			else
			{
				MappingPropertyState->State = StateMappingProperty::EState::ExpectKey;
			}

			return Ok();
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
			return Fail(EErrorCode::ReadStructKeyFail);	
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
				return Fail(EErrorCode::ReadStructFail);
			}
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return Fail(EErrorCode::ReadStructAfterEnded);
		}
		checkNoEntry();
	}
	else if (StateMappingProperty* MapPropertyState = TryGetTopState<StateMappingProperty>(this))
	{
		if (MapPropertyState->State == StateMappingProperty::EState::ExpectKey)
		{
			//	FMapOfStruct2
			if (UStructProperty* StructProperty = Cast<UStructProperty>(MapPropertyState->MapHelper.GetKeyProperty()))
			{
				void* StructPtr = MapPropertyState->MapHelper.GetKeyPtr(MapPropertyState->Index);
				PushStructRootState(this,
					StructPtr,
					StructProperty->Struct
				);
				StateStructRoot& StructRootRef = GetTopState<StateStructRoot>(this);
				PushFirstStructPropertyState(this, OutNamePtr, StructRootRef.StructPtr, StructRootRef.StructClass);

				return Ok();
			}

			return Fail(EErrorCode::ReadStructFail);
		}
		else if (MapPropertyState->State == StateMappingProperty::EState::ExpectValue)
		{
			if (UStructProperty* StructProperty = Cast<UStructProperty>(MapPropertyState->MapHelper.GetValueProperty()))
			{
				void* StructPtr = MapPropertyState->MapHelper.GetValuePtr(MapPropertyState->Index);

				PushStructRootState(this,
					StructPtr,
					StructProperty->Struct
				);
				StateStructRoot& StructRootRef = GetTopState<StateStructRoot>(this);
				PushFirstStructPropertyState(this, OutNamePtr, StructRootRef.StructPtr, StructRootRef.StructClass);

				return Ok();
			}
			else
			{
				return Fail(EErrorCode::ReadStructFail);
			}
		}
		else
		{
			return Fail(EErrorCode::ReadMapEndFail);
		}
	}
	else if (StateStructRoot* StructRootState = TryGetTopState<StateStructRoot>(this))
	{
		PushFirstStructPropertyState(this, OutNamePtr, StructRootState->StructPtr, StructRootState->StructClass);
		return Ok();
	}

	return Fail(EErrorCode::ReadStructFail);
}

FResult FPropertyReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State != StateStructProperty::EState::Ended)
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
		return Fail(EErrorCode::ReadStructEndFail);
	}
}

FResult FPropertyReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	if (StateStructProperty* StructPropertyState = TryGetTopState<StateStructProperty>(this))
	{
		if (StructPropertyState->State == StateStructProperty::EState::ExpectKey)
		{
			return Fail(EErrorCode::ReadMapFail);
		}
		else if (StructPropertyState->State == StateStructProperty::EState::ExpectValue)
		{
			if (UMapProperty* MapProperty = Cast<UMapProperty>(StructPropertyState->Property))
			{
				void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(StructPropertyState->StructPtr);
				PushMappingRootState(this, MapPtr, MapProperty);
				PushMappingPropertyState(this, MapPtr, MapProperty);

				return Ok();
			}
			else
			{
				return Fail(EErrorCode::ReadMapFail);
			}
		}
		else if (StructPropertyState->State == StateStructProperty::EState::Ended)
		{
			return Fail(EErrorCode::ReadStructAfterEnded);
		}
	}
	else if (StateMappingRoot* MappingRootState = TryGetTopState<StateMappingRoot>(this))
	{
		PushMappingPropertyState(this, MappingRootState->MapPtr, MappingRootState->MapProperty);
		return Ok();
	}

	return Fail(EErrorCode::ReadMapFail);
}

FResult FPropertyReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (StateMappingProperty* MappingPropertyState = TryGetTopState<StateMappingProperty>(this))
	{
		if (MappingPropertyState->State != StateMappingProperty::EState::Ended)
		{
			return Fail(EErrorCode::ReadMapEndWhenStillHasValue);
		}

		PopState(this);
		PopState(this);

		return FinishTopStateValueRead(this);
	}

	return Ok();
}

FPropertyReader::FPropertyState::FPropertyState()
{
	static_assert(sizeof(FPropertyState) <= 64, "larger than cacheline");
	static_assert(sizeof(ReaderState) <= sizeof(FPropertyState), "impl storage size too small");
	//	TODO drop the TVariant and use a pod type, as we know it's a pod
	new(&ImplStorage) ReaderState(TInPlaceType<StateNil>{});
}



} // namespace DataConfig
