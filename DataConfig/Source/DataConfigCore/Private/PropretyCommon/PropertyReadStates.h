#pragma once

#include "UObject/UnrealType.h"

namespace DataConfig {

enum class EPropertyType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
};

enum class EDataEntry;
struct FContextStorage;

struct FBaseState
{
	//	RTTI alternative
	virtual EPropertyType GetType() = 0;

	virtual EDataEntry Peek();
	virtual FResult ReadName(FName* OutNamePtr, FContextStorage* CtxPtr);
	virtual FResult ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum);

	//	intentionally ommitting virtual destructor, we don't really need it as
	//	we want all these states bitwise relocateable

	template<typename T>
	T* As();
};

template<typename T>
T* FBaseState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

template<typename TState, typename TStorage, typename... TArgs>
void Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
	new (Storage) TState(Forward<TArgs>(Args)...);
}

struct FStateNil : public FBaseState
{
	static const EPropertyType ID = EPropertyType::Nil;

	EPropertyType GetType() override;
};

struct FStateClass : public FBaseState
{
	static const EPropertyType ID = EPropertyType::ClassProperty;

	UObject* ClassObject;
	UProperty* Property;

	FStateClass(UObject* InClassObject)
	{
		ClassObject = InClassObject;
		Property = nullptr;
	}

	EPropertyType GetType() override;
};

struct FStateStruct : public FBaseState
{
	static const EPropertyType ID = EPropertyType::StructProperty;

	void* StructPtr;
	UScriptStruct* StructClass;
	UProperty* Property;

	enum class EState
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		Ended
	};

	EState State;

	FStateStruct(void* InStructPtr, UScriptStruct* InStructClass)
	{
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EPropertyType GetType() override;
	EDataEntry Peek() override;
	FResult ReadName(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum) override;

	FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr);

};

struct FStateMap : public FBaseState
{
	static const EPropertyType ID = EPropertyType::MapProperty;

	void* MapPtr;
	UMapProperty* MapProperty;
	uint16 Index;

	FStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
	}

	EPropertyType GetType() override;
};

//	storage is already POD type, and TArray<> do only bitwise relocate anyway
//	we'll just needs to assume these types are trivially destructable
static_assert(TIsTriviallyDestructible<FStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FStateMap>::Value, "need trivial destructible");

} // namespace DataConfig





