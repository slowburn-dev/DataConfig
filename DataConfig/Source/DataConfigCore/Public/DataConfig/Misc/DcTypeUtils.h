#pragma once

#include "DataConfig/DcTypes.h"

namespace DcTypeUtils {

struct FDcInvalidType {};

FORCEINLINE bool IsNumericDataEntry(const EDcDataEntry& Entry)
{
	return Entry == EDcDataEntry::Int8
		|| Entry == EDcDataEntry::Int16
		|| Entry == EDcDataEntry::Int32
		|| Entry == EDcDataEntry::Int64
		|| Entry == EDcDataEntry::UInt8
		|| Entry == EDcDataEntry::UInt16
		|| Entry == EDcDataEntry::UInt32
		|| Entry == EDcDataEntry::UInt64
		|| Entry == EDcDataEntry::Float
		|| Entry == EDcDataEntry::Double;
}

template <typename TLhs, typename TRhs>
struct TIsSameSize
{
	enum { Value = sizeof(TLhs) == sizeof(TRhs) };
};

static_assert(TIsSameSize<uint8, int8>::Value, "yes");
static_assert(!TIsSameSize<uint8, int32>::Value, "no");

template<typename T>
struct TDcDataEntryType
{
	static constexpr EDcDataEntry Value = EDcDataEntry::Ended;
};

template<> struct TDcDataEntryType<nullptr_t> { static constexpr EDcDataEntry Value = EDcDataEntry::Nil; };
template<> struct TDcDataEntryType<bool> { static constexpr EDcDataEntry Value = EDcDataEntry::Bool; };
template<> struct TDcDataEntryType<FName> { static constexpr EDcDataEntry Value = EDcDataEntry::Name; };
template<> struct TDcDataEntryType<FString> { static constexpr EDcDataEntry Value = EDcDataEntry::String; };
template<> struct TDcDataEntryType<FText> { static constexpr EDcDataEntry Value = EDcDataEntry::Text; };
template<> struct TDcDataEntryType<FDcEnumData> { static constexpr EDcDataEntry Value = EDcDataEntry::Enum; };

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

//	FMemoryReader doesn't support `UObject`, check `MemoryArchive.h`
//	might need a special pair of Reader/Writer for variant
/*
template<> struct TDcDataEntryType<UObject*> { static constexpr EDcDataEntry Value = EDcDataEntry::ObjectReference; };
template<> struct TDcDataEntryType<UClass*> { static constexpr EDcDataEntry Value = EDcDataEntry::ClassReference; };
*/
//	TODO might need to .. support the whole data model?


//	TODO if this isn't used in the end, just delete it
template<EDcDataEntry Entry>
struct TDcTypeToDataEntry
{
	using Type = FDcInvalidType;
};

template<> struct TDcTypeToDataEntry<EDcDataEntry::Nil> { using Type = nullptr_t; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Bool> { using Type = bool; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Name> { using Type = FName; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::String> { using Type = FString; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Text> { using Type = FText; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Enum> { using Type = FDcEnumData; };

template<> struct TDcTypeToDataEntry<EDcDataEntry::Float> { using Type = float; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Double> { using Type = double; };

template<> struct TDcTypeToDataEntry<EDcDataEntry::Int8> { using Type = int8; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Int16> { using Type = int16; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Int32> { using Type = int32; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::Int64> { using Type = int64; };

template<> struct TDcTypeToDataEntry<EDcDataEntry::UInt8> { using Type = uint8; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::UInt16> { using Type = uint16; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::UInt32> { using Type = uint32; };
template<> struct TDcTypeToDataEntry<EDcDataEntry::UInt64> { using Type = uint64; };

FORCEINLINE FString SafeNameToString(const FName& Value)
{
	return Value.IsValid() ? Value.ToString() : TEXT("<invalid-name>");
}


}	// namespace FDcTypeUtils

