#pragma once

#include "UObject/UnrealType.h"

enum class EDcPropertyWriteType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
};


enum class EDcDataEntry;

struct FDcBaseWriteState
{
	virtual EDcPropertyWriteType GetType() = 0;

	virtual FDcResult Peek(EDcDataEntry Next);
	virtual FDcResult WriteName(const FName& Value);
	virtual FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);
	virtual FDcResult SkipWrite();
	virtual FDcResult PeekWriteProperty(UField** OutProperty);

	virtual void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType);

	template<typename T>
	T* As();

	//	non copyable
	FDcBaseWriteState() = default;
	FDcBaseWriteState(const FNoncopyable&) = delete;
	FDcBaseWriteState& operator=(const FDcBaseReadState&) = delete;
};

template<typename T>
T* FDcBaseWriteState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

struct FDcWriteStateNil : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::Nil;

	FDcWriteStateNil() = default;

	EDcPropertyWriteType GetType() override;
	FDcResult Peek(EDcDataEntry Next) override;

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateStruct : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::StructProperty;

	FName StructName;
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

	FDcWriteStateStruct(void* InStructPtr, UScriptStruct* InStructClass, const FName& InStructName)
	{
		StructName = InStructName;
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteStructRoot(const FName& Name);
	FDcResult WriteStructEnd(const FName& Name);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateClass : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::ClassProperty;

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

	FDcWriteStateClass(UObject* InClassObject, UClass* InClass)
	{
		Datum.DataPtr = InClassObject;
		Datum.Property = InClass;
		Class = InClass;
		State = EState::ExpectRoot;
		Type = EType::Root;
	}

	FDcWriteStateClass(void* DataPtr, UObjectProperty* InObjProperty)
	{
		Datum.DataPtr = DataPtr;
		Datum.Property = InObjProperty;
		Class = InObjProperty->PropertyClass;
		State = EState::ExpectRoot;
		Type = EType::PropertyNormalOrInstanced;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteNil();
	FDcResult WriteClassRoot(const FDcClassPropertyStat& Class);
	FDcResult WriteClassEnd(const FDcClassPropertyStat& Class);
	FDcResult WriteReference(UObject* Value);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateMap : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::MapProperty;

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

	FDcWriteStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
		bNeedsRehash = false;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteMapRoot();
	FDcResult WriteMapEnd();

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateArray : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::ArrayProperty;

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

	FDcWriteStateArray(void* InArrayPtr, UArrayProperty* InArrayProperty)
	{
		ArrayPtr = InArrayPtr;
		ArrayProperty = InArrayProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult Peek(EDcDataEntry Next) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteArrayRoot();
	FDcResult WriteArrayEnd();

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

template<typename TProperty, typename TValue>
FDcResult WriteValue(FDcBaseWriteState& State, const TValue& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(State.WriteDataEntry(TProperty::StaticClass(), Datum));

	Datum.CastChecked<TProperty>()->SetPropertyValue(Datum.DataPtr, Value);
	return DcOk();
}

