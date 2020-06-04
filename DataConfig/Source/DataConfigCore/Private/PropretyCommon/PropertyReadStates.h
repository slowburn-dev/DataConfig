#pragma once

#include "UObject/UnrealType.h"
#include "Reader/PropertyReader.h"

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
	virtual FResult EndReadValue();

	//	!!!  intentionally ommitting virtual destructor, keep these state trivia
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
TState& Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
	return *(new (Storage) TState(Forward<TArgs>(Args)...));
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

	enum class EState
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		Ended,
	};
	EState State;

	FStateClass(UObject* InClassObject)
	{
		ClassObject = InClassObject;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EPropertyType GetType() override;
	EDataEntry Peek() override;
	FResult ReadName(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum) override;
	FResult EndReadValue() override;

	FResult ReadClassRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	FResult ReadClassEnd(FName* OutNamePtr, FContextStorage* CtxPtr);
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
	FResult EndReadValue() override;

	FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr);
	FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr);
};

struct FStateMap : public FBaseState
{
	static const EPropertyType ID = EPropertyType::MapProperty;

	void* MapPtr;
	UMapProperty* MapProperty;
	uint16 Index;

	enum class EState : uint16
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		Ended
	};

	EState State;

	FStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
	}

	EPropertyType GetType() override;
	EDataEntry Peek() override;
	FResult ReadName(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadDataEntry(UClass* ExpectedPropertyClass, EErrorCode FailCode, FContextStorage* CtxPtr, FPropertyDatum& OutDatum) override;
	FResult EndReadValue() override;

	FResult ReadMapRoot(FContextStorage* CtxPtr);
	FResult ReadMapEnd(FContextStorage* CtxPtr);
};

//	storage is already POD type, and TArray<> do only bitwise relocate anyway
//	we'll just needs to assume these types are trivially destructable
static_assert(TIsTriviallyDestructible<FStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FStateMap>::Value, "need trivial destructible");

//	need these as readers needs to push states
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

FStateNil& PushNilState(FPropertyReader* Reader);
FStateClass& PushClassPropertyState(FPropertyReader* Reader, UObject* InClassObject);
FStateStruct& PushStructPropertyState(FPropertyReader* Reader, void* InStructPtr, UScriptStruct* InStructClass);
FStateMap& PushMappingPropertyState(FPropertyReader* Reader, void* InMapPtr, UMapProperty* InMapProperty);
void PopState(FPropertyReader* Reader);

template<typename TState>
static void PopState(FPropertyReader* Reader)
{
	check(TState::ID == GetTopState(Reader).GetType());
	PopState(Reader);
}

} // namespace DataConfig





