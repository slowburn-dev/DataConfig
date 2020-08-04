#pragma once

#include "UObject/UnrealType.h"

namespace DataConfig {

enum class EPropertyWriteType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
};


enum class EDataEntry;

struct FBaseWriteState
{
	virtual EPropertyWriteType GetType() = 0;

	virtual FResult Peek(EDataEntry Next);
	virtual FResult WriteName(const FName& Value);
	virtual FResult WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum);
	virtual FResult SkipWrite();
	virtual FResult PeekWriteProperty(UField** OutProperty);

	template<typename T>
	T* As();

	//	non copyable
	FBaseWriteState() = default;
	FBaseWriteState(const FNoncopyable&) = delete;
	FBaseWriteState& operator=(const FBaseReadState&) = delete;
};

template<typename T>
T* FBaseWriteState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

struct FWriteStateNil : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::Nil;

	FWriteStateNil() = default;

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;

};

struct FWriteStateStruct : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::StructProperty;

	void* StructPtr;
	UScriptStruct* StructClass;
	UProperty* Property;

	enum class EState
	{
		ExpectRoot,
		ExpectKeyOrEnd,
		ExpectValue,
		Ended,
	};
	EState State;

	FWriteStateStruct(void* InStructPtr, UScriptStruct* InStructClass)
	{
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum) override;
	FResult SkipWrite() override;
	FResult PeekWriteProperty(UField** OutProperty) override;

	FResult WriteStructRoot(const FName& Name);
	FResult WriteStructEnd(const FName& Name);

};

struct FWriteStateClass : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::ClassProperty;

	FPropertyDatum Datum;
	UClass* Class;

	enum class EState : uint16
	{
		ExpectRoot,
		ExpectNil,
		ExpectReference,
		ExpectKeyOrEnd,
		ExpectValue,
		Ended,
	};
	EState State;

	enum class EType : uint16
	{
		Root,
		PropertyNormalOrInstanced,
		//	TODO soft
	};
	EType Type;

	FWriteStateClass(UObject* InClassObject, UClass* InClass)
	{
		Datum.DataPtr = InClassObject;
		Datum.Property = InClass;
		Class = InClass;
		State = EState::ExpectRoot;
		Type = EType::Root;
	}

	FWriteStateClass(void* DataPtr, UObjectProperty* InObjProperty)
	{
		Datum.DataPtr = DataPtr;
		Datum.Property = InObjProperty;
		Class = InObjProperty->PropertyClass;
		State = EState::ExpectRoot;
		Type = EType::PropertyNormalOrInstanced;
	}

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum) override;
	FResult SkipWrite() override;
	FResult PeekWriteProperty(UField** OutProperty) override;

	FResult WriteNil();
	FResult WriteClassRoot(const FClassPropertyStat& Class);
	FResult WriteClassEnd(const FClassPropertyStat& Class);
	FResult WriteReference(UObject* Value);
};

struct FWriteStateMap : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::MapProperty;

	void* MapPtr;
	UMapProperty* MapProperty;
	uint16 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectKeyOrEnd,
		ExpectValue,
		Ended
	};
	EState State;

	bool bNeedsRehash;

	FWriteStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
		bNeedsRehash = false;
	}

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum) override;
	FResult SkipWrite() override;
	FResult PeekWriteProperty(UField** OutProperty) override;

	FResult WriteMapRoot();
	FResult WriteMapEnd();

};

struct FWriteStateArray : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::ArrayProperty;

	void* ArrayPtr;
	UArrayProperty* ArrayProperty;
	uint16 Index;

	enum class EState : uint16
	{
		ExpectRoot,
		ExpectItemOrEnd,
		Ended,
	};
	EState State;

	FWriteStateArray(void* InArrayPtr, UArrayProperty* InArrayProperty)
	{
		ArrayPtr = InArrayPtr;
		ArrayProperty = InArrayProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EPropertyWriteType GetType() override;
	FResult Peek(EDataEntry Next) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteDataEntry(UClass* ExpectedPropertyClass, FPropertyDatum& OutDatum) override;
	FResult SkipWrite() override;
	FResult PeekWriteProperty(UField** OutProperty) override;

	FResult WriteArrayRoot();
	FResult WriteArrayEnd();
};

template<typename TProperty, typename TValue>
FResult WriteValue(FBaseWriteState& State, const TValue& Value)
{
	FPropertyDatum Datum;
	TRY(State.WriteDataEntry(TProperty::StaticClass(), Datum));

	Datum.CastChecked<TProperty>()->SetPropertyValue(Datum.DataPtr, Value);
	return Ok();
}


} // namespace DataConfig


