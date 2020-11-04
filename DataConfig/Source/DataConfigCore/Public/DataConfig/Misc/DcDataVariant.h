#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "DataConfig/DcTypes.h"

template<typename T>
struct TDcDataEntryType
{
	static constexpr EDcDataEntry Value = EDcDataEntry::Ended;
};

template<> struct TDcDataEntryType<nullptr_t> { static constexpr EDcDataEntry Value = EDcDataEntry::Nil; };
template<> struct TDcDataEntryType<bool> { static constexpr EDcDataEntry Value = EDcDataEntry::Bool; };
template<> struct TDcDataEntryType<FName> { static constexpr EDcDataEntry Value = EDcDataEntry::Name; };
template<> struct TDcDataEntryType<FString> { static constexpr EDcDataEntry Value = EDcDataEntry::String; };

template<> struct TDcDataEntryType<float> { static constexpr EDcDataEntry Value = EDcDataEntry::Float; };
template<> struct TDcDataEntryType<double> { static constexpr EDcDataEntry Value = EDcDataEntry::Double; };

template<> struct TDcDataEntryType<int8> { static constexpr EDcDataEntry Value = EDcDataEntry::Int8; };
template<> struct TDcDataEntryType<int16> { static constexpr EDcDataEntry Value = EDcDataEntry::Int16; };
template<> struct TDcDataEntryType<int32> { static constexpr EDcDataEntry Value = EDcDataEntry::Int32; };
template<> struct TDcDataEntryType<int64> { static constexpr EDcDataEntry Value = EDcDataEntry::Int64; };

template<> struct TDcDataEntryType<uint8> { static constexpr EDcDataEntry Value = EDcDataEntry::UInt8; };
template<> struct TDcDataEntryType<uint16> { static constexpr EDcDataEntry Value = EDcDataEntry::UInt16; };
template<> struct TDcDataEntryType<uint32> { static constexpr EDcDataEntry Value = EDcDataEntry::UInt32; };
template<> struct TDcDataEntryType<uint64> { static constexpr EDcDataEntry Value = EDcDataEntry::UInt64; };


template<typename T>
struct TDcIsDataVariantCompatible
{
	enum { Value = TDcDataEntryType<T>::Value != EDcDataEntry::Ended };
};

static_assert(TDcIsDataVariantCompatible<int>::Value, "yes");
static_assert(!TDcIsDataVariantCompatible<EDcDataEntry>::Value, "no");


struct FDcDataVariant
{
	FDcDataVariant()
		: DataType(EDcDataEntry::Nil)
	{}

	FDcDataVariant(const FDcDataVariant&) = default;
	FDcDataVariant& operator=(const FDcDataVariant&) = default;
	~FDcDataVariant() = default;
	FDcDataVariant(FDcDataVariant&&) = default;
	FDcDataVariant& operator=(FDcDataVariant&&) = default;

	template<typename T>
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
		using TActual = typename TRemoveConst<TRemoveReference<T>::Type>::Type;
		DataType = TDcDataEntryType<TActual>::Value;

		FMemoryWriter MemWriter(Value, true);
		MemWriter << InValue;
	}

	template<>
	FORCEINLINE void Initialize<nullptr_t>(nullptr_t InValue)
	{
		DataType = EDcDataEntry::Nil;
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
		check(TDcDataEntryType<T>::Value == DataType);

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
	TArray<uint8> Value;
};


static_assert(sizeof(FDcDataVariant) <= 64, "data variant too large");


