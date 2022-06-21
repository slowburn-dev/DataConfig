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
struct FDcPropertyReader;

struct FDcBaseReadState
{
	virtual EDcPropertyReadType GetType() = 0;

	virtual FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr);
	virtual FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr);
	virtual FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);
	virtual FDcResult SkipRead(FDcPropertyReader* Parent);
	virtual FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty);
	virtual FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr);

	virtual void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) = 0;

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
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
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
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;

	FDcResult ReadClassRootAccess(FDcPropertyReader* Parent, FDcClassAccess& Access);
	FDcResult ReadClassEndAccess(FDcPropertyReader* Parent, FDcClassAccess& Access);
	FDcResult ReadNil(FDcPropertyReader* Parent);
	FDcResult ReadObjectReference(FDcPropertyReader* Parent, UObject** OutPtr);
	void EndValueRead(FDcPropertyReader* Parent);
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
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;

	FDcResult ReadStructRootAccess(FDcPropertyReader* Parent, FDcStructAccess& Access);
	FDcResult ReadStructEndAccess(FDcPropertyReader* Parent, FDcStructAccess& Access);

	void EndValueRead(FDcPropertyReader* Parent);
};

struct FDcReadStateMap : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::MapProperty;


	enum class EState : uint8
	{
		ExpectRoot,
		ExpectKey,
		ExpectValue,
		ExpectEnd,
		Ended
	};
	EState State = EState::ExpectRoot;
	int32 Index = 0;
	int32 SparseIndex = 0;

	FName MapName;
	FScriptMapHelper MapHelper;
	FMapProperty* MapProperty;

	FDcReadStateMap(void* InMapPtr, FMapProperty* InMapProperty)
		: MapHelper(InMapProperty, InMapPtr)
	{
		MapName = InMapProperty->GetFName();
		MapProperty = InMapProperty;
	}

	FDcReadStateMap(FProperty* InKeyProperty, FProperty* InValueProperty, void* InMapPtr, EMapPropertyFlags InMapFlags)
		: MapHelper(FScriptMapHelper::CreateHelperFormInnerProperties(InKeyProperty, InValueProperty, InMapPtr, InMapFlags))
	{
		MapName = DC_TRANSIENT_MAP;
		MapProperty = nullptr;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;

	FDcResult ReadMapRoot(FDcPropertyReader* Parent);
	FDcResult ReadMapEnd(FDcPropertyReader* Parent);
};

struct FDcReadStateArray : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::ArrayProperty;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State = EState::ExpectRoot;
	int32 Index = 0;

	FName ArrayName;
	FScriptArrayHelper ArrayHelper;
	FArrayProperty* ArrayProperty;

	FDcReadStateArray(void* InArrayPtr, FArrayProperty* InArrayProperty)
		: ArrayHelper(InArrayProperty, InArrayPtr)
	{
		ArrayName = InArrayProperty->GetFName();
		ArrayProperty = InArrayProperty;
	}

	FDcReadStateArray(FProperty* InInnerProperty, void *InArray, EArrayPropertyFlags InArrayFlags)
		: ArrayHelper(FScriptArrayHelper::CreateHelperFormInnerProperty(InInnerProperty, InArray, InArrayFlags))
	{
		ArrayName = DC_TRANSIENT_ARRAY;
		ArrayProperty = nullptr;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;

	FDcResult ReadArrayRoot(FDcPropertyReader* Parent);
	FDcResult ReadArrayEnd(FDcPropertyReader* Parent);
};

struct FDcReadStateSet : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::SetProperty;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectEnd,
		ExpectItem,
		Ended,
	};
	EState State = EState::ExpectRoot;
	int32 Index = 0;
	int32 SparseIndex = 0;

	FName SetName;
	FScriptSetHelper SetHelper;
	FSetProperty* SetProperty;

	FDcReadStateSet(void* InSetPtr, FSetProperty* InSetProperty)
		: SetHelper(InSetProperty, InSetPtr)
	{
		SetName = InSetProperty->GetFName();
		SetProperty = InSetProperty;
	}

	FDcReadStateSet(FProperty* InInnerProperty, void* InSet)
		: SetHelper(FScriptSetHelper::CreateHelperFormElementProperty(InInnerProperty, InSet))
	{
		SetName = DC_TRANSIENT_SET;
		SetProperty = nullptr;
	}

	EDcPropertyReadType GetType() override;

	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;

	FDcResult ReadSetRoot(FDcPropertyReader* Parent);
	FDcResult ReadSetEnd(FDcPropertyReader* Parent);
};

struct FDcReadStateScalar : public FDcBaseReadState
{
	static const EDcPropertyReadType ID = EDcPropertyReadType::ScalarProperty;

	enum class EState : uint8
	{
		//    ArrayDim == 1
		ExpectScalar,    

		//    ArrayDim > 1
		ExpectArrayRoot,
		ExpectArrayItem,
		ExpectArrayEnd,

		Ended,
	};
	EState State;
	int32 Index;

	FProperty* ScalarField;
	void* ScalarPtr;

	enum EArrayScalar { Array };

	FDcReadStateScalar(void* InPtr, FProperty* InField)
	{
		ScalarField = InField;
		ScalarPtr = InPtr;
		Index = 0;

		State = ScalarField->ArrayDim > 1
			? EState::ExpectArrayRoot
			: EState::ExpectScalar;
	}

	FDcReadStateScalar(EArrayScalar, void* InPtr, FProperty* InField)
	{
		check(DcPropertyUtils::IsScalarArray(InField));
		ScalarField = InField;
		ScalarPtr = InPtr;
		State = EState::ExpectArrayRoot;
		Index = 0;
	}

	EDcPropertyReadType GetType() override;
	FDcResult PeekRead(FDcPropertyReader* Parent, EDcDataEntry* OutPtr) override;
	FDcResult ReadName(FDcPropertyReader* Parent, FName* OutNamePtr) override;
	FDcResult ReadDataEntry(FDcPropertyReader* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult PeekReadProperty(FDcPropertyReader* Parent, FFieldVariant* OutProperty) override;
	FDcResult PeekReadDataPtr(FDcPropertyReader* Parent, void** OutDataPtr) override;
	FDcResult SkipRead(FDcPropertyReader* Parent) override;

	FDcResult ReadArrayRoot(FDcPropertyReader* Parent);
	FDcResult ReadArrayEnd(FDcPropertyReader* Parent);

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


