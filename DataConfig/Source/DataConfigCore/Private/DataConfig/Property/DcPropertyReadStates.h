#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyStatesCommon.h"

enum class EDcPropertyReadType
{
	Nil,
	ClassProperty,
	StructProperty,
	MapProperty,
	ArrayProperty,
	SetProperty,
	ScalarProperty,
};

enum class EDcDataEntry : uint16;
struct FDcPropertyDatum;

struct FDcBaseReadState
{
	virtual EDcPropertyReadType GetType() = 0;

	virtual FDcResult PeekRead(EDcDataEntry* OutPtr);
	virtual FDcResult ReadName(FName* OutNamePtr);
	virtual FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);
	virtual FDcResult SkipRead();
	virtual FDcResult PeekReadProperty(FFieldVariant* OutProperty);
	virtual FDcResult PeekReadDataPtr(void** OutDataPtr);

	virtual void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType);

	template<typename T>
	T* As();

	//	explicit disable copy. can't use FNonCopyable as it makes destructor non trivia
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
	FProperty* Property;

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
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead() override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	FDcResult ReadClassRootAccess(FDcClassAccess& Access);
	FDcResult ReadClassEndAccess(FDcClassAccess& Access);
	FDcResult ReadNil();
	FDcResult ReadObjectReference(UObject** OutPtr);
	void EndValueRead();
};

struct FDcReadStateStruct : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::StructProperty;

	FName StructName;
	void* StructPtr;
	UScriptStruct* StructClass;
	FProperty* Property;

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
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead() override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	FDcResult ReadStructRootAccess(FDcStructAccess& Access);
	FDcResult ReadStructEndAccess(FDcStructAccess& Access);

	void EndValueRead();
};

struct FDcReadStateMap : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::MapProperty;

	void* MapPtr;
	FMapProperty* MapProperty;
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

	FDcReadStateMap(void* InMapPtr, FMapProperty* InMapProperty)
	{
		MapPtr = InMapPtr;
		MapProperty = InMapProperty;
		Index = 0;
		State = EState::ExpectRoot;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead() override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	FDcResult ReadMapRoot();
	FDcResult ReadMapEnd();
};

struct FDcReadStateArray : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::ArrayProperty;

	void* ArrayPtr;
	FArrayProperty* ArrayProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State;

	FDcReadStateArray(void* InArrayPtr, FArrayProperty* InArrayProperty)
	{
		ArrayPtr = InArrayPtr;
		ArrayProperty = InArrayProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead() override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	FDcResult ReadArrayRoot();
	FDcResult ReadArrayEnd();
};

struct FDcReadStateSet : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::SetProperty;

	void* SetPtr;
	FSetProperty* SetProperty;
	int32 Index;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State;

	FDcReadStateSet(void* InSetPtr, FSetProperty* InSetProperty)
	{
		SetPtr = InSetPtr;
		SetProperty = InSetProperty;
		State = EState::ExpectRoot;
		Index = 0;
	}

	EDcPropertyReadType GetType() override;

	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead() override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	FDcResult ReadSetRoot();
	FDcResult ReadSetEnd();
};

struct FDcReadStateScalar : public FDcBaseReadState
{
	enum class EState : uint16
	{
		ExpectRead,
		Ended,
	};
	EState State;

	FField* ScalarField;
	void* ScalarPtr;

	FDcReadStateScalar(void* InPtr, FField* InField)
	{
		State = EState::ExpectRead;
		ScalarField = InField;
		ScalarPtr = InPtr;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult PeekReadProperty(FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(void** OutDataPtr) override;

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

//	storage is already POD type, and TArray<> do only bitwise relocate anyway
//	we'll just needs to assume these types are trivially destructable
static_assert(TIsTriviallyDestructible<FDcReadStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateMap>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateArray>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateSet>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcReadStateScalar>::Value, "need trivial destructible");


