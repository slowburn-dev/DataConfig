#pragma once

#include "DataConfig/DcTypes.h"

namespace DcTypeUtils {

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

template<typename T>
struct TIsUClass
{
	template <typename C> static uint16 Test(decltype(C::StaticClass));
	template <typename C> static uint8 Test(...);
	
	enum { Value = sizeof(Test<T>(0)) - 1};
};
	
template<typename T>
struct TIsUStruct
{
	template <typename C> static uint16 Test(decltype(C::StaticStruct));
	template <typename C> static uint8 Test(...);
	
	enum { Value = sizeof(Test<T>(0)) - 1};
};

}	// namespace FDcTypeUtils

