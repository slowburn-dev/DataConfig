#pragma once

#include "DataConfig/DcTypes.h"
#include "Misc/EngineVersionComparison.h"

namespace DcTypeUtils {

FORCEINLINE bool IsNumericDataEntry(const EDcDataEntry& Entry)
{
	switch (Entry)
	{
	case EDcDataEntry::Int8:
	case EDcDataEntry::Int16:
	case EDcDataEntry::Int32:
	case EDcDataEntry::Int64:
	case EDcDataEntry::UInt8:
	case EDcDataEntry::UInt16:
	case EDcDataEntry::UInt32:
	case EDcDataEntry::UInt64:
	case EDcDataEntry::Float:
	case EDcDataEntry::Double:
		return true;
	default:
		return false;
	}
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

template<> struct TDcDataEntryType<nullptr_t> { static constexpr EDcDataEntry Value = EDcDataEntry::None; };
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
struct TIsDataEntryType
{
	enum { Value = TDcDataEntryType<T>::Value != EDcDataEntry::Ended };
};

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

template<typename A, typename B>
struct TIsSame
{
#if UE_VERSION_OLDER_THAN(5, 2, 0)
	enum { Value = ::TIsSame<A, B>::Value };
#else
	enum { Value = std::is_same_v<A, B>	};
#endif
};

template<typename T>
struct TRemoveConst
{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	using Type = typename ::TRemoveConst<T>::Type;
#else
	using Type = std::remove_const_t<T>;
#endif
};


} // namespace DcTypeUtils

