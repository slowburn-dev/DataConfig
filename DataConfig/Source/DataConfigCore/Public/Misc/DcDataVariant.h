#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "DcTypes.h"

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

	FDataVariant(const TCHAR* InString)
	{
		*this = FString(InString);
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

	EDataEntry DataType;
	TArray<uint8> Value;
};


static_assert(sizeof(FDataVariant) <= 64, "data variant too large");


} // namespace DataConfig








