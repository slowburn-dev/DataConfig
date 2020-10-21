#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "DataConfig/DcTypes.h"

namespace DataConfig
{

template<typename T>
struct TDcDataEntryType
{
	static constexpr EDataEntry Value = EDataEntry::Ended;
};

template<> struct TDcDataEntryType<nullptr_t> { static constexpr EDataEntry Value = EDataEntry::Nil; };
template<> struct TDcDataEntryType<bool> { static constexpr EDataEntry Value = EDataEntry::Bool; };
template<> struct TDcDataEntryType<FName> { static constexpr EDataEntry Value = EDataEntry::Name; };
template<> struct TDcDataEntryType<FString> { static constexpr EDataEntry Value = EDataEntry::String; };

template<> struct TDcDataEntryType<float> { static constexpr EDataEntry Value = EDataEntry::Float; };
template<> struct TDcDataEntryType<double> { static constexpr EDataEntry Value = EDataEntry::Double; };

template<> struct TDcDataEntryType<int8> { static constexpr EDataEntry Value = EDataEntry::Int8; };
template<> struct TDcDataEntryType<int16> { static constexpr EDataEntry Value = EDataEntry::Int16; };
template<> struct TDcDataEntryType<int32> { static constexpr EDataEntry Value = EDataEntry::Int; };
template<> struct TDcDataEntryType<int64> { static constexpr EDataEntry Value = EDataEntry::Int64; };

template<> struct TDcDataEntryType<uint8> { static constexpr EDataEntry Value = EDataEntry::Byte; };
template<> struct TDcDataEntryType<uint16> { static constexpr EDataEntry Value = EDataEntry::UInt16; };
template<> struct TDcDataEntryType<uint32> { static constexpr EDataEntry Value = EDataEntry::UInt32; };
template<> struct TDcDataEntryType<uint64> { static constexpr EDataEntry Value = EDataEntry::UInt64; };


template<typename T>
struct TDcIsDataVariantCompatible
{
	enum { Value = TDcDataEntryType<T>::Value != EDataEntry::Ended };
};

static_assert(TDcIsDataVariantCompatible<int>::Value, "yes");
static_assert(!TDcIsDataVariantCompatible<EDataEntry>::Value, "no");


struct FDcDataVariant
{
	FDcDataVariant()
		: DataType(EDataEntry::Nil)
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
		DataType = EDataEntry::Nil;
	}

	FDcDataVariant(const TCHAR* InString)
	{
		*this = FString(InString);
	}

	FDcDataVariant(TCHAR InTChar)
	{
		*this = FString(1, &InTChar);
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
		check(DataType == EDataEntry::Nil);
		return nullptr;
	}

	EDataEntry DataType;
	TArray<uint8> Value;
};


static_assert(sizeof(FDcDataVariant) <= 64, "data variant too large");


} // namespace DataConfig








