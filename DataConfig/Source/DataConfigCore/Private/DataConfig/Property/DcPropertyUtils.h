#pragma once

class UProperty;
class UStruct;

bool IsEffectiveProperty(UProperty* Property);

size_t CountEffectiveProperties(UStruct* Struct);

UProperty* NextEffectiveProperty(UProperty* Property);
UProperty* FirstEffectiveProperty(UProperty* Property);
UProperty* NextPropertyByName(UProperty* InProperty, const FName& Name);

enum class EDcDataEntry;
EDcDataEntry PropertyToDataEntry(UField* Property);

template<typename TState, typename TStorage, typename... TArgs>
TState& Emplace(TStorage* Storage, TArgs&&... Args)
{
	static_assert(sizeof(TState) <= sizeof(TStorage), "storage too small");
	return *(new (Storage) TState(Forward<TArgs>(Args)...));
}

//	Cpp type to Property
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

//	Proprety to string
static FString GetFormatPropertyName(UProperty* Property);


