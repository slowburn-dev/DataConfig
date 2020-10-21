#pragma once

#include "UObject/UnrealType.h"

enum class EPropertyWriteType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
};


enum class EDcDataEntry;

struct FBaseWriteState
{
	virtual EPropertyWriteType GetType() = 0;

	virtual FDcResult Peek(EDcDataEntry Next);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);
	virtual FDcResult SkipWrite();
	virtual FDcResult PeekWriteProperty(UField** OutProperty);

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
	FDcResult Peek(EDcDataEntry Next) override;

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
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteStructRoot(const FName& Name);
	FDcResult WriteStructEnd(const FName& Name);

};

struct FWriteStateClass : public FBaseWriteState
{
	static const EPropertyWriteType ID = EPropertyWriteType::ClassProperty;

	FDcPropertyDatum Datum;
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
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteNil();
	FDcResult WriteClassRoot(const FDcClassPropertyStat& Class);
	FDcResult WriteClassEnd(const FDcClassPropertyStat& Class);
	FDcResult WriteReference(UObject* Value);
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
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteMapRoot();
	FDcResult WriteMapEnd();

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
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteArrayRoot();
	FDcResult WriteArrayEnd();
};

template<typename TProperty, typename TValue>
FDcResult WriteValue(FBaseWriteState& State, const TValue& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(State.WriteDataEntry(TProperty::StaticClass(), Datum));

	Datum.CastChecked<TProperty>()->SetPropertyValue(Datum.DataPtr, Value);
	return DcOk();
}

