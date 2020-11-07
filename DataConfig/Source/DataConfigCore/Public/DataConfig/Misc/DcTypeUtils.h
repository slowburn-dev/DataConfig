#pragma once

#include "DataConfig/DcTypes.h"

namespace FDcTypeUtils {

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
	struct TPropertyTypeMap
	{};

	template<> struct TPropertyTypeMap<bool> { using Type = UBoolProperty; };
	template<> struct TPropertyTypeMap<FName> { using Type = UNameProperty; };
	template<> struct TPropertyTypeMap<FString> { using Type = UStrProperty; };

	template<> struct TPropertyTypeMap<int8> { using Type = UInt8Property; };
	template<> struct TPropertyTypeMap<int16> { using Type = UInt16Property; };
	template<> struct TPropertyTypeMap<int32> { using Type = UIntProperty; };
	template<> struct TPropertyTypeMap<int64> { using Type = UInt64Property; };

	template<> struct TPropertyTypeMap<uint8> { using Type = UByteProperty; };
	template<> struct TPropertyTypeMap<uint16> { using Type = UUInt16Property; };
	template<> struct TPropertyTypeMap<uint32> { using Type = UUInt32Property; };
	template<> struct TPropertyTypeMap<uint64> { using Type = UUInt64Property; };

	template<> struct TPropertyTypeMap<float> { using Type = UFloatProperty; };
	template<> struct TPropertyTypeMap<double> { using Type = UDoubleProperty; };

	static_assert(TIsSame<TPropertyTypeMap<int32>::Type, UIntProperty>::Value, "yes");

}	// namespace FDcTypeUtils

