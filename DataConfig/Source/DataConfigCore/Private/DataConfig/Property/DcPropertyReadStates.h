#pragma once

#include "UObject/UnrealType.h"
#include "DataConfig/Property/DcPropertyUtils.h"

enum class EDcPropertyReadType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
	SetProperty,
};

enum class EDcDataEntry : uint16;

struct FDcBaseReadState
{
	virtual EDcPropertyReadType GetType() = 0;

	virtual FDcResult PeekRead(EDcDataEntry* OutPtr);
	virtual FDcResult ReadName(FName* OutNamePtr);
	virtual FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	virtual void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType);

	//	!!!  intentionally ommitting virtual destructor, keep these state trivia
	template<typename T>
	T* As();

	//	non copyable
	FDcBaseReadState() = default;
	FDcBaseReadState(const FNoncopyable&) = delete;
	FDcBaseReadState& operator=(const FDcBaseReadState&) = delete;
};

template<typename T>
T* FDcBaseReadState::As()
{
	if (GetType() == T::ID)
		return (T*)this;
	else
		return nullptr;
}

struct FDcReadStateNil : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::Nil;

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcReadStateClass : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::ClassProperty;

	FName ObjectName;
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
	};
	EType Type;

	FDcReadStateClass(UObject* InClassObject, UClass* InClass, EType InType, const FName& InObjectName)
	{
		ObjectName = InObjectName;
		ClassObject = InClassObject;
		Class = InClass;
		Property = nullptr;
		State = EState::ExpectRoot;
		Type = InType;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult EndReadValue();	// TODO remove this method, called only in 1 place
	FDcResult ReadClassRoot(FDcObjectPropertyStat* OutClassPtr);
	FDcResult ReadClassEnd(FDcObjectPropertyStat* OutClassPtr);
	FDcResult ReadNil();
	FDcResult ReadObjectReference(UObject** OutPtr);

};

struct FDcReadStateStruct : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::StructProperty;

	FName StructName;
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

	FDcReadStateStruct(void* InStructPtr, UScriptStruct* InStructClass, const FName& InStructName)
	{
		StructName = InStructName;
		StructPtr = InStructPtr;
		StructClass = InStructClass;
		Property = nullptr;
		State = EState::ExpectRoot;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult EndReadValue();
	FDcResult ReadStructRoot(FName* OutNamePtr);
	FDcResult ReadStructEnd(FName* OutNamePtr);
};

struct FDcReadStateMap : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::MapProperty;

	void* MapPtr;
	UMapProperty* MapProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		Ended
	};

	EState State;

	FDcReadStateMap(void* InMapPtr, UMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult EndReadValue();
	FDcResult ReadMapRoot();
	FDcResult ReadMapEnd();
};

struct FDcReadStateArray : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::ArrayProperty;

	void* ArrayPtr;
	UArrayProperty* ArrayProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State;

	FDcReadStateArray(void* InArrayPtr, UArrayProperty* InArrayProperty)
	{
		ArrayPtr = InArrayPtr;
		ArrayProperty = InArrayProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult EndReadValue(); // TODO remove this method, called only in 1 place
	FDcResult ReadArrayRoot();
	FDcResult ReadArrayEnd();
};

struct FDcReadStateSet : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::SetProperty;

	void* SetPtr;
	USetProperty* SetProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State;

	FDcReadStateSet(void* InSetPtr, USetProperty* InSetProperty)
	{
		SetPtr = InSetPtr;
		SetProperty = InSetProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EDcPropertyReadType GetType() override;

	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult ReadSetRoot();
	FDcResult ReadSetEnd();
};


//	storage is already POD type, and TArray<> do only bitwise relocate anyway
//	we'll just needs to assume these types are trivially destructable
static_assert(TIsTriviallyDestructible<FDcReadStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateMap>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateArray>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateSet>::Value, "need trivial destructible");






