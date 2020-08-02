#include "DataConfig/Property/DcPropertyReader.h"
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "DataConfig/DcErrorCodes.h"
#include "DataConfig/Property/DcPropertyReadStates.h"
#include "DataConfig/Property/DcPropertyUtils.h"

namespace DataConfig {

//	need these as readers needs to push states
using ReaderStorageType = FPropertyReader::FPropertyState::ImplStorageType;

static FORCEINLINE ReaderStorageType* GetTopStorage(FPropertyReader* Self)
{
	return &Self->States.Top().ImplStorage;
}

static FORCEINLINE FBaseReadState& GetTopState(FPropertyReader* Self)
{
	return *reinterpret_cast<FBaseReadState*>(GetTopStorage(Self));
}

template<typename TState>
static TState& GetTopState(FPropertyReader* Self) {
	return *GetTopState(Self).As<TState>();
}

template<typename TState>
static TState* TryGetTopState(FPropertyReader* Self) {
	return GetTopState(Self).As<TState>();
}

FReadStateNil& PushNilState(FPropertyReader* Reader)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateNil>(GetTopStorage(Reader));
}

FReadStateClass& PushClassPropertyState(FPropertyReader* Reader, UObject* InClassObject, UClass* InClass, FReadStateClass::EType InType)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateClass>(GetTopStorage(Reader), InClassObject, InClass, InType);
}

FReadStateStruct& PushStructPropertyState(FPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateStruct>(GetTopStorage(Reader), InStructPtr, InStructClass);
}

FReadStateMap& PushMappingPropertyState(FPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateMap>(GetTopStorage(Reader), InMapPtr, InMapProperty);
}

FReadStateArray& PushArrayPropertyState(FPropertyReader* Reader, void* InArrayPtr, UArrayProperty* InArrayProperty)
{
	Reader->States.AddDefaulted();
	return Emplace<FReadStateArray>(GetTopStorage(Reader), InArrayPtr, InArrayProperty);
}

void PopState(FPropertyReader* Reader)
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


template<typename TProperty, typename TPrimitive, EErrorCode ErrCode>
FORCEINLINE FResult ReadTopStateProperty(FPropertyReader* Self, TPrimitive* OutPtr, FContextStorage* CtxPtr)
{
	FPropertyDatum Datum;
	TRY(GetTopState(Self).ReadDataEntry(TProperty::StaticClass(), ErrCode, CtxPtr, Datum));

	if (OutPtr)
	{
		*OutPtr = Datum.CastChecked<TProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
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
	else if (Datum.Property->IsA<UClass>())
	{
		UObject* Obj = (UObject*)(Datum.DataPtr);
		check(IsValid(Obj));
		PushClassPropertyState(this, Obj, Datum.CastChecked<UClass>(), FReadStateClass::EType::Root);
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
	return ReadTopStateProperty<UBoolProperty, bool, EErrorCode::ReadBoolFail>(this, OutPtr, CtxPtr);
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
		*OutPtr = Datum.CastChecked<UStrProperty>()->GetPropertyValue(Datum.DataPtr);
	}

	return Ok();
}

FResult FPropertyReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateStruct* StructState = TopState.As<FReadStateStruct>();
		if (StructState != nullptr
			&& StructState->State == FReadStateStruct::EState::ExpectRoot)
		{
			TRY(StructState->ReadStructRoot(OutNamePtr, CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UStructProperty::StaticClass(), EErrorCode::ReadStructFail, CtxPtr, Datum));

		FReadStateStruct& ChildStruct = PushStructPropertyState(this, Datum.DataPtr, Datum.CastChecked<UStructProperty>()->Struct);
		TRY(ChildStruct.ReadStructRoot(OutNamePtr, CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr)
{
	if (FReadStateStruct* StructState = TryGetTopState<FReadStateStruct>(this))
	{
		TRY(StructState->ReadStructEnd(OutNamePtr, CtxPtr));
		PopState<FReadStateStruct>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateStruct::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyReader::ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateClass* ClassState = TopState.As<FReadStateClass>();
		if (ClassState != nullptr
			&& ClassState->State == FReadStateClass::EState::ExpectRoot)
		{
			TRY(ClassState->ReadClassRoot(OutClassPtr, CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UObjectProperty::StaticClass(), EErrorCode::ReadClassFail, CtxPtr, Datum));

		UObjectProperty* ObjProperty = Datum.CastChecked<UObjectProperty>();
		check(ObjProperty);
		FReadStateClass& ChildClass = PushClassPropertyState(
			this,
			ObjProperty->GetObjectPropertyValue(Datum.DataPtr),
			ObjProperty->PropertyClass,
			ObjProperty->HasAnyPropertyFlags(CPF_InstancedReference)
				? FReadStateClass::EType::PropertyInstanced
				: FReadStateClass::EType::PropertyNormal
		);
		TRY(ChildClass.ReadClassRoot(OutClassPtr, CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr)
{
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		TRY(ClassState->ReadClassEnd(OutClassPtr, CtxPtr));
		PopState<FReadStateClass>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	FBaseReadState& TopState = GetTopState(this);
	{
		FReadStateMap* MapState = TopState.As<FReadStateMap>();
		if (MapState != nullptr
			&& MapState->State == FReadStateMap::EState::ExpectRoot)
		{
			TRY(MapState->ReadMapRoot(CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UMapProperty::StaticClass(), EErrorCode::ReadMapFail, CtxPtr, Datum));

		FReadStateMap& ChildMap = PushMappingPropertyState(this, Datum.DataPtr, Datum.CastChecked<UMapProperty>());
		TRY(ChildMap.ReadMapRoot(CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	if (FReadStateMap* StateMap = TryGetTopState<FReadStateMap>(this))
	{
		TRY(StateMap->ReadMapEnd(CtxPtr));
		PopState<FReadStateMap>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateMap::ID << (int)GetTopState(this).GetType();
	}
}

FResult FPropertyReader::ReadArrayRoot(FContextStorage* CtxPtr)
{
	FBaseReadState& TopState = GetTopState(this);

	{
		FReadStateArray* ArrayState = TopState.As<FReadStateArray>();
		if (ArrayState != nullptr
			&& ArrayState->State == FReadStateArray::EState::ExpectRoot)
		{
			TRY(ArrayState->ReadArrayRoot(CtxPtr));
			return Ok();
		}
	}

	{
		FPropertyDatum Datum;
		TRY(TopState.ReadDataEntry(UArrayProperty::StaticClass(), EErrorCode::ReadArrayFail, CtxPtr, Datum));

		FReadStateArray& ChildArray = PushArrayPropertyState(this, Datum.DataPtr, Datum.CastChecked<UArrayProperty>());
		TRY(ChildArray.ReadArrayRoot(CtxPtr));
	}

	return Ok();
}

FResult FPropertyReader::ReadArrayEnd(FContextStorage* CtxPtr)
{
	if (FReadStateArray* ArrayState = TryGetTopState<FReadStateArray>(this))
	{
		TRY(ArrayState->ReadArrayEnd(CtxPtr));
		PopState<FReadStateArray>(this);
		return Ok();
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateArray::ID << (int)GetTopState(this).GetType();

	}
}


DataConfig::FResult FPropertyReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr)
{
	//	only class property reads reference
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		return ClassState->ReadReference(OutPtr, CtxPtr);
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();

	}
}

FResult FPropertyReader::ReadNil(FContextStorage* CtxPtr)
{
	//	only class property accepts nil
	if (FReadStateClass* ClassState = TryGetTopState<FReadStateClass>(this))
	{
		return ClassState->ReadNil(CtxPtr);
	}
	else
	{
		return Fail(DIAG(DReadWrite, InvalidStateWithExpect))
			<< (int)FReadStateClass::ID << (int)GetTopState(this).GetType();
	}
}

} // namespace DataConfig
