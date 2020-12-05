#pragma once

#include "UObject/UnrealType.h"

enum class EDcPropertyWriteType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
	SetProperty,
};

enum class EDcDataEntry : uint16;

struct FDcBaseWriteState
{
	virtual EDcPropertyWriteType GetType() = 0;

	virtual FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk);
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
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;

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
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;
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

	FName ObjectName;
	UObject* ClassObject;
	UClass* Class;
	FDcPropertyDatum Datum;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectReference,
		ExpectEnd,
		ExpectExpandKeyOrEnd,
		ExpectExpandValue,
		Ended,
	};
	EState State;

	enum class EType : uint8
	{
		Root,
		PropertyNormalOrInstanced,
	};
	EType Type;

	FDcWriteStateClass(UObject* InClassObject, UClass* InClass)
	{
		ObjectName = InClassObject->GetFName();
		Class = InClass;
		ClassObject = InClassObject;
		Datum.DataPtr = InClassObject;
		Datum.Property = InClass;
		State = EState::ExpectRoot;
		Type = EType::Root;
	}

	FDcWriteStateClass(void* DataPtr, UObjectProperty* InObjProperty)
	{
		ObjectName = InObjProperty->GetFName();
		Class = InObjProperty->PropertyClass;
		//	note that atm can be null, referencing something, or even uninitialized
		ClassObject = nullptr;
		Datum.DataPtr = DataPtr;
		Datum.Property = InObjProperty;
		State = EState::ExpectRoot;
		Type = EType::PropertyNormalOrInstanced;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteNil();
	FDcResult WriteClassRoot(const FDcObjectPropertyStat& Class);
	FDcResult WriteClassEnd(const FDcObjectPropertyStat& Class);
	FDcResult WriteObjectReference(const UObject* Value);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateMap : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::MapProperty;

	void* MapPtr;
	UMapProperty* MapProperty;
	int32 Index;

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
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;
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
	int32 Index;

	enum class EState : uint8
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
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteArrayRoot();
	FDcResult WriteArrayEnd();

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateSet : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::SetProperty;

	void *SetPtr;
	USetProperty* SetProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectItemOrEnd,
		Ended,
	};
	EState State;

	bool bNeedsRehash;

	FDcWriteStateSet(void* InSetPtr, USetProperty* InSetProperty)
	{
		SetPtr = InSetPtr;
		SetProperty = InSetProperty;
		State = EState::ExpectRoot;
		Index = 0;
		bNeedsRehash = false;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite() override;
	FDcResult PeekWriteProperty(UField** OutProperty) override;

	FDcResult WriteSetRoot();
	FDcResult WriteSetEnd();

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

template<typename TProperty, typename TValue>
FDcResult WriteValue(FDcBaseWriteState& State, const TValue& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(State.WriteDataEntry(TProperty::StaticClass(), Datum));

	WritePropertyValueConversion<TProperty, TValue>(Datum.Property, Datum.DataPtr, Value);
	return DcOk();
}

static_assert(TIsTriviallyDestructible<FDcWriteStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateMap>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateArray>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateSet>::Value, "need trivial destructible");

