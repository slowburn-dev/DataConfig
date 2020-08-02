#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "DataConfig/DcTypes.h"

namespace DataConfig
{

template<typename T>
struct TDataEntryType
{
	static constexpr EDataEntry Value;
};

template<> struct TDataEntryType<nullptr_t> { static constexpr EDataEntry Value = EDataEntry::Nil; };
template<> struct TDataEntryType<bool> { static constexpr EDataEntry Value = EDataEntry::Bool; };
template<> struct TDataEntryType<FName> { static constexpr EDataEntry Value = EDataEntry::Name; };
template<> struct TDataEntryType<FString> { static constexpr EDataEntry Value = EDataEntry::String; };

template<> struct TDataEntryType<float> { static constexpr EDataEntry Value = EDataEntry::Float; };
template<> struct TDataEntryType<double> { static constexpr EDataEntry Value = EDataEntry::Double; };

template<> struct TDataEntryType<int8> { static constexpr EDataEntry Value = EDataEntry::Int8; };
template<> struct TDataEntryType<int16> { static constexpr EDataEntry Value = EDataEntry::Int16; };
template<> struct TDataEntryType<int32> { static constexpr EDataEntry Value = EDataEntry::Int; };
template<> struct TDataEntryType<int64> { static constexpr EDataEntry Value = EDataEntry::Int64; };

template<> struct TDataEntryType<uint8> { static constexpr EDataEntry Value = EDataEntry::Byte; };
template<> struct TDataEntryType<uint16> { static constexpr EDataEntry Value = EDataEntry::UInt16; };
template<> struct TDataEntryType<uint32> { static constexpr EDataEntry Value = EDataEntry::UInt32; };
template<> struct TDataEntryType<uint64> { static constexpr EDataEntry Value = EDataEntry::UInt64; };


struct FDataVariant
{
	FDataVariant()
		: DataType(EDataEntry::Nil)
	{}

	FDataVariant(const FDataVariant&) = default;
	FDataVariant& operator=(const FDataVariant&) = default;
	~FDataVariant() = default;
	FDataVariant(FDataVariant&&) = default;
	FDataVariant& operator=(FDataVariant&&) = default;

	template<typename T>
	FDataVariant(T&& InValue)
	{
		Initialize(Forward<T>(InValue));
	}

	template<typename T>
	FDataVariant& operator=(T&& InValue)
	{
		Initialize(Forward<T>(InValue));
		return *this;
	}

	template<typename T>
	FORCEINLINE void Initialize(T InValue)
	{
		using TActual = typename TRemoveConst<TRemoveReference<T>::Type>::Type;
		DataType = TDataEntryType<TActual>::Value;

		FMemoryWriter MemWriter(Value, true);
		MemWriter << InValue;
	}

	template<>
	FORCEINLINE void Initialize<nullptr_t>(nullptr_t InValue)
	{
		DataType = EDataEntry::Nil;
	}

	FDataVariant(const TCHAR* InString)
	{
		*this = FString(InString);
	}

	FDataVariant(TCHAR InTChar)
	{
		*this = FString(1, &InTChar);
	}

	template<typename T>
	T GetValue() const
	{
		check(TDataEntryType<T>::Value == DataType);

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


static_assert(sizeof(FDataVariant) <= 64, "data variant too large");


} // namespace DataConfig








