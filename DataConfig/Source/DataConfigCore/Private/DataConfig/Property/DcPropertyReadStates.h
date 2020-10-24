#pragma once

#include "UObject/UnrealType.h"

enum class EPropertyReadType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
};

enum class EDcDataEntry;

struct FBaseReadState
{
	virtual EPropertyReadType GetType() = 0;

	virtual FDcResult PeekRead(EDcDataEntry* OutPtr);
	virtual FDcResult ReadName(FName* OutNamePtr);
	virtual FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	//	!!!  intentionally ommitting virtual destructor, keep these state trivia
	template<typename T>
	T* As();

	//	non copyable
	FBaseReadState() = default;
	FBaseReadState(const FNoncopyable&) = delete;
	FBaseReadState& operator=(const FBaseReadState&) = delete;
};

template<typename T>
T* FBaseReadState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

struct FReadStateNil : public FBaseReadState
{
	static const EPropertyReadType ID = EPropertyReadType::Nil;

	EPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
};

struct FReadStateClass : public FBaseReadState
{
	static const EPropertyReadType ID = EPropertyReadType::ClassProperty;

	UObject* ClassObject;
	UClass* Class;
	UProperty* Property;

	enum class EState : uint16
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		ExpectNil,
		ExpectReference,
		Ended,
	};
	EState State;

	enum class EType : uint16
	{
		Root,
		PropertyNormal,
		PropertyInstanced,
		//	TODO soft
	};
	EType Type;

	FReadStateClass(UObject* InClassObject, UClass* InClass, EType InType)
	{
		ClassObject = InClassObject;
		Class = InClass;
		Property = nullptr;
		State = EState::ExpectRoot;
		Type = InType;
	}

	EPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;

	FDcResult EndReadValue();
	FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr);
	FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr);
	FDcResult ReadNil();
	FDcResult ReadReference(UObject** OutPtr);

};

struct FReadStateStruct : public FBaseReadState
{
	static const EPropertyReadType ID = EPropertyReadType::StructProperty;

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

	FReadStateStruct(void* InStructPtr, UScriptStruct* InStructClass)
	{
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;

	FDcResult EndReadValue();
	FDcResult ReadStructRoot(FName* OutNamePtr);
	FDcResult ReadStructEnd(FName* OutNamePtr);
};

struct FReadStateMap : public FBaseReadState
{
	static const EPropertyReadType ID = EPropertyReadType::MapProperty;

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

	FReadStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
	}

	EPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;

	FDcResult EndReadValue();
	FDcResult ReadMapRoot();
	FDcResult ReadMapEnd();
};

struct FReadStateArray : public FBaseReadState
{
	static const EPropertyReadType ID = EPropertyReadType::ArrayProperty;

	void* ArrayPtr;
	UArrayProperty* ArrayProperty;
	uint16 Index;

	enum class EState : uint16
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State;

	FReadStateArray(void* InArrayPtr, UArrayProperty* InArrayProperty)
	{
		ArrayPtr = InArrayPtr;
		ArrayProperty = InArrayProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;

	FDcResult EndReadValue();
	FDcResult ReadArrayRoot();
	FDcResult ReadArrayEnd();
};

//	storage is already POD type, and TArray<> do only bitwise relocate anyway
//	we'll just needs to assume these types are trivially destructable
static_assert(TIsTriviallyDestructible<FReadStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FReadStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FReadStateMap>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FReadStateArray>::Value, "need trivial destructible");






