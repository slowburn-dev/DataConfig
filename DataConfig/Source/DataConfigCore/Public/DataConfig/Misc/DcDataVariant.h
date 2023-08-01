#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcTypeUtils.h"

template<typename T>
struct TDcIsDataVariantCompatible
{
	enum { Value = 
		DcTypeUtils::TIsDataEntryType<T>::Value
		|| TIsEnum<T>::Value
	};
};

template<> struct TDcIsDataVariantCompatible<EDcDataEntry>
{
	enum { Value = true };
};

static_assert(TDcIsDataVariantCompatible<int>::Value, "yes");
static_assert(!TDcIsDataVariantCompatible<FDcStructAccess>::Value, "no");

struct FDcDataVariant
{
	FDcDataVariant()
		: DataType(EDcDataEntry::Nil)
	{}

	FDcDataVariant(const FDcDataVariant&) = default;
	FDcDataVariant& operator=(const FDcDataVariant&) = default;
	FDcDataVariant(FDcDataVariant&&) = default;
	FDcDataVariant& operator=(FDcDataVariant&&) = default;
	~FDcDataVariant() = default;

	template<
		typename T,
		typename TActual = typename DcTypeUtils::TRemoveConst<typename TRemoveReference<T>::Type>::Type,
		typename X = typename TEnableIf<TDcIsDataVariantCompatible<TActual>::Value, void>::Type
	>
	FDcDataVariant(T&& InValue)
	{
		Initialize(Forward<T>(InValue));
	}

	template<typename T>
	FDcDataVariant& operator=(T&& InValue)
	{
		Initialize(Forward<T>(InValue));
		return *this;
	}

	template<typename T>
	FORCEINLINE void Initialize(T InValue)
	{
		using TActual = typename DcTypeUtils::TRemoveConst<typename TRemoveReference<T>::Type>::Type;
		DataType = DcTypeUtils::TDcDataEntryType<TActual>::Value;
		bDataTypeOnly = false;

		FMemoryWriter MemWriter(Value, true);
		MemWriter << InValue;
	}

	template<>
	FORCEINLINE void Initialize<nullptr_t>(nullptr_t)
	{
		DataType = EDcDataEntry::Nil;
		bDataTypeOnly = false;
	}

	template<>
	FORCEINLINE void Initialize<EDcDataEntry>(EDcDataEntry InDataEntry)
	{
		DataType = InDataEntry;
		bDataTypeOnly = true;
	}

	FDcDataVariant(const WIDECHAR* InString)
	{
		*this = FString(InString);
	}

	FDcDataVariant(WIDECHAR InChar)
	{
		*this = FString(1, &InChar);
	}

	FDcDataVariant(const ANSICHAR* InString)
	{
		*this = FString(InString);
	}

	FDcDataVariant(ANSICHAR InChar)
	{
		*this = FString(1, &InChar);
	}

	template<typename T>
	T GetValue() const
	{
		static_assert(DcTypeUtils::TDcDataEntryType<T>::Value != EDcDataEntry::Ended, "[DataConfig] unsupported T type");

		check(DcTypeUtils::TDcDataEntryType<T>::Value == DataType);
		T Result;

		FMemoryReader Reader(Value, true);
		Reader << Result;

		return Result;
	}

	//	specialize for null as archive doesn't support this
	template<>
	nullptr_t GetValue<nullptr_t>() const
	{
		check(DataType == EDcDataEntry::Nil);
		return nullptr;
	}

	EDcDataEntry DataType;
	bool bDataTypeOnly;
	TArray<uint8> Value;
};


static_assert(sizeof(FDcDataVariant) <= 64, "data variant too large");


