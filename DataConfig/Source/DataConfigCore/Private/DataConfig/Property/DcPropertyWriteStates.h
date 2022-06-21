#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyStatesCommon.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "UObject/UnrealType.h"

enum class EDcPropertyWriteType
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
struct FDcPropertyWriter;

struct FDcBaseWriteState
{
	virtual EDcPropertyWriteType GetType() = 0;

	virtual FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk);
	virtual FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value);
	virtual FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);
	virtual FDcResult SkipWrite(FDcPropertyWriter* Parent);
	virtual FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty);

	virtual void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) = 0;

	template<typename T>
	T* As();

	//	explicit disable copy. can't use FNonCopyable as it makes destructor non trivia
	FDcBaseWriteState() = default;
	FDcBaseWriteState(const FNoncopyable&) = delete;
	FDcBaseWriteState& operator=(const FDcBaseWriteState&) = delete;
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
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateStruct : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::StructProperty;

	FName StructName;
	void* StructPtr;
	UScriptStruct* StructClass;
	FProperty* Property;

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
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteStructRootAccess(FDcPropertyWriter* Parent, FDcStructAccess& Access);
	FDcResult WriteStructEndAccess(FDcPropertyWriter* Parent, FDcStructAccess& Access);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateClass : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::ClassProperty;

	UObject* ClassObject;
	UClass* Class;

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

	FDcClassAccess::EControl ConfigControl;

	FName ObjectName;
	FDcPropertyDatum Datum;

	FDcWriteStateClass(UObject* InClassObject, UClass* InClass)
	{
		ObjectName = InClassObject->GetFName();
		Class = InClass;
		ClassObject = InClassObject;
		Datum.DataPtr = InClassObject;
		Datum.Property = InClass;
		State = EState::ExpectRoot;
		Type = EType::Root;
		ConfigControl = FDcClassAccess::EControl::ExpandObject;
	}

	FDcWriteStateClass(void* DataPtr, FObjectProperty* InObjProperty, FDcClassAccess::EControl InConfigControl)
	{
		ObjectName = InObjProperty->GetFName();
		//	note that `Class` can be rewrite to more derived type later
		Class = InObjProperty->PropertyClass;
		//	note that atm can be null, referencing something, or even uninitialized
		ClassObject = nullptr;
		Datum.DataPtr = DataPtr;
		Datum.Property = InObjProperty;
		State = EState::ExpectRoot;
		Type = EType::PropertyNormalOrInstanced;
		ConfigControl = InConfigControl;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteNil(FDcPropertyWriter* Parent);
	FDcResult WriteClassRootAccess(FDcPropertyWriter* Parent, FDcClassAccess& Access);
	FDcResult WriteClassEndAccess(FDcPropertyWriter* Parent, FDcClassAccess& Access);
	FDcResult WriteObjectReference(FDcPropertyWriter* Parent, const UObject* Value);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateMap : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::MapProperty;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectKeyOrEnd,
		ExpectValue,
		Ended
	};
	EState State = EState::ExpectRoot;
	bool bNeedsRehash = false;
	int32 Index = 0;

	FName MapName;
	FScriptMapHelper MapHelper;
	FMapProperty* MapProperty;

	FDcWriteStateMap(void* InMapPtr, FMapProperty* InMapProperty)
		: MapHelper(InMapProperty, InMapPtr)
	{
		MapName = InMapProperty->GetFName();	
		MapProperty = InMapProperty;
	}

	FDcWriteStateMap(FProperty* InKeyProperty, FProperty* InValueProperty, void* InMapPtr, EMapPropertyFlags InMapFlags)
		: MapHelper(FScriptMapHelper::CreateHelperFormInnerProperties(InKeyProperty, InValueProperty, InMapPtr, InMapFlags))
	{
		MapName = DC_TRANSIENT_MAP;
		MapProperty = nullptr;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteMapRoot(FDcPropertyWriter* Parent);
	FDcResult WriteMapEnd(FDcPropertyWriter* Parent);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateArray : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::ArrayProperty;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectItemOrEnd,
		Ended,
	};
	EState State = EState::ExpectRoot;
	int32 Index = 0;

	FName ArrayName;
	FScriptArrayHelper ArrayHelper;
	FArrayProperty* ArrayProperty;

	FDcWriteStateArray(void* InArrayPtr, FArrayProperty* InArrayProperty)
		: ArrayHelper(InArrayProperty, InArrayPtr)
	{
		ArrayName = InArrayProperty->GetFName();
		ArrayProperty = InArrayProperty;
	}

	FDcWriteStateArray(FProperty* InInnerProperty, void* InArray, EArrayPropertyFlags InArrayFlags)
		: ArrayHelper(FScriptArrayHelper::CreateHelperFormInnerProperty(InInnerProperty, InArray, InArrayFlags))
	{
		ArrayName = DC_TRANSIENT_ARRAY;
		ArrayProperty = nullptr;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteArrayRoot(FDcPropertyWriter* Parent);
	FDcResult WriteArrayEnd(FDcPropertyWriter* Parent);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};

struct FDcWriteStateSet : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::SetProperty;

	enum class EState : uint8
	{
		ExpectRoot,
		ExpectItemOrEnd,
		Ended,
	};
	EState State = EState::ExpectRoot;
	bool bNeedsRehash = false;
	int32 Index = 0;

	FName SetName;
	FScriptSetHelper SetHelper;
	FSetProperty* SetProperty;

	FDcWriteStateSet(void* InSetPtr, FSetProperty* InSetProperty)
		: SetHelper(InSetProperty, InSetPtr)
	{
		SetName = InSetProperty->GetFName();
		SetProperty = InSetProperty;
	}

	FDcWriteStateSet(FProperty* InInnerProperty, void* InSet)
		: SetHelper(FScriptSetHelper::CreateHelperFormElementProperty(InInnerProperty, InSet))
	{
		SetName = DC_TRANSIENT_SET;
		SetProperty = nullptr;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteSetRoot(FDcPropertyWriter* Parent);
	FDcResult WriteSetEnd(FDcPropertyWriter* Parent);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};


struct FDcWriteStateScalar : public FDcBaseWriteState
{
	static const EDcPropertyWriteType ID = EDcPropertyWriteType::ScalarProperty;

	enum class EState : uint16
	{
		//	ArrayDim == 1
		ExpectScalar,

		//	ArrayDim > 1
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

	FDcWriteStateScalar(void* InPtr, FProperty* InField)
	{
		ScalarField = InField;
		ScalarPtr = InPtr;
		Index = 0;

		State = ScalarField->ArrayDim > 1
			? EState::ExpectArrayRoot
			: EState::ExpectScalar;
	}

	FDcWriteStateScalar(EArrayScalar, void* InPtr, FProperty* InField)
	{
		check(DcPropertyUtils::IsScalarArray(InField));
		ScalarField = InField;
		ScalarPtr = InPtr;
		State = EState::ExpectArrayRoot;
		Index = 0;
	}

	EDcPropertyWriteType GetType() override;
	FDcResult PeekWrite(FDcPropertyWriter* Parent, EDcDataEntry Next, bool* bOutOk) override;
	FDcResult WriteName(FDcPropertyWriter* Parent, const FName& Value) override;
	FDcResult WriteDataEntry(FDcPropertyWriter* Parent, FFieldClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum) override;
	FDcResult SkipWrite(FDcPropertyWriter* Parent) override;
	FDcResult PeekWriteProperty(FDcPropertyWriter* Parent, FFieldVariant* OutProperty) override;

	FDcResult WriteArrayRoot(FDcPropertyWriter* Parent);
	FDcResult WriteArrayEnd(FDcPropertyWriter* Parent);

	void FormatHighlightSegment(TArray<FString>& OutSegments, DcPropertyHighlight::EFormatSeg SegType) override;
};


template<typename TProperty, typename TScalar>
FORCEINLINE void WritePropertyValueConversion(FField* Property, void* Ptr, const TScalar& Value)
{
	CastFieldChecked<TProperty>(Property)->SetPropertyValue(Ptr, Value);
}

template<typename TProperty, typename TValue>
FDcResult WriteValue(FDcPropertyWriter* Parent, FDcBaseWriteState& State, const TValue& Value)
{
	FDcPropertyDatum Datum;
	DC_TRY(State.WriteDataEntry(Parent, TProperty::StaticClass(), Datum));

	check(!Datum.Property.IsUObject());
	WritePropertyValueConversion<TProperty, TValue>(Datum.Property.ToFieldUnsafe(), Datum.DataPtr, Value);
	return DcOk();
}

static_assert(TIsTriviallyDestructible<FDcWriteStateClass>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateStruct>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateMap>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateArray>::Value, "need trivial destructible");
static_assert(TIsTriviallyDestructible<FDcWriteStateSet>::Value, "need trivial destructible");

