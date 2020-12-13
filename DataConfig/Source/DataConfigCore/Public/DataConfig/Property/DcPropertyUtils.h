#pragma once

class FProperty;
class UStruct;
enum class EDcDataEntry : uint16;

namespace DcPropertyUtils
{

bool IsEffectiveProperty(FProperty* Property);
bool IsScalarProperty(FField* Property);

size_t CountEffectiveProperties(UStruct* Struct);

FProperty* NextEffectiveProperty(FProperty* Property);
FProperty* FirstEffectiveProperty(FProperty* Property);
FProperty* NextPropertyByName(FProperty* InProperty, const FName& Name);

EDcDataEntry PropertyToDataEntry(const FFieldVariant& Field);
EDcDataEntry PropertyToDataEntry(FField* Property);

FString GetFormatPropertyTypeName(FField* Property);
FString GetFormatPropertyTypeName(UScriptStruct* Struct);
FString GetFormatPropertyTypeName(UClass* Class);
FString GetFormatPropertyTypeName(const FFieldVariant& Field);

//	Cpp type to Property
template<typename T>
struct TPropertyTypeMap
{};

template<> struct TPropertyTypeMap<bool> { using Type = FBoolProperty; };
template<> struct TPropertyTypeMap<FName> { using Type = FNameProperty; };
template<> struct TPropertyTypeMap<FString> { using Type = FStrProperty; };
template<> struct TPropertyTypeMap<FText> { using Type = FTextProperty; };

template<> struct TPropertyTypeMap<int8> { using Type = FInt8Property; };
template<> struct TPropertyTypeMap<int16> { using Type = FInt16Property; };
template<> struct TPropertyTypeMap<int32> { using Type = FIntProperty; };
template<> struct TPropertyTypeMap<int64> { using Type = FInt64Property; };

template<> struct TPropertyTypeMap<uint8> { using Type = FByteProperty; };
template<> struct TPropertyTypeMap<uint16> { using Type = FUInt16Property; };
template<> struct TPropertyTypeMap<uint32> { using Type = FUInt32Property; };
template<> struct TPropertyTypeMap<uint64> { using Type = FUInt64Property; };

template<> struct TPropertyTypeMap<float> { using Type = FFloatProperty; };
template<> struct TPropertyTypeMap<double> { using Type = FDoubleProperty; };

template<> struct TPropertyTypeMap<UClass*> { using Type = FClassProperty; };
template<> struct TPropertyTypeMap<UObject*> { using Type = FObjectProperty; };

template<> struct TPropertyTypeMap<FWeakObjectPtr> { using Type = FWeakObjectProperty; };
template<> struct TPropertyTypeMap<FLazyObjectPtr> { using Type = FLazyObjectProperty; };
template<> struct TPropertyTypeMap<FSoftObjectPath> { using Type = FSoftObjectProperty; };
template<> struct TPropertyTypeMap<FSoftClassPath> { using Type = FSoftClassProperty; };
template<> struct TPropertyTypeMap<FScriptInterface> { using Type = FInterfaceProperty; };

template<> struct TPropertyTypeMap<FScriptDelegate> { using Type = FDelegateProperty; };
template<> struct TPropertyTypeMap<FMulticastScriptDelegate> { using Type = FMulticastInlineDelegateProperty; };
template<> struct TPropertyTypeMap<FSparseDelegate> { using Type = FMulticastSparseDelegateProperty; };

static_assert(TIsSame<TPropertyTypeMap<int32>::Type, FIntProperty>::Value, "yes");

template<typename T>
FORCEINLINE T* CastFieldVariant(FFieldVariant& FieldVariant)
{
	static_assert(TIsDerivedFrom<T, FField>::Value, "expect TProperty to be a field");
	return ::CastField<T>(FieldVariant.ToField());
}

template<>
FORCEINLINE UClass* CastFieldVariant<UClass>(FFieldVariant& FieldVariant)
{
	return ::Cast<UClass>(FieldVariant.ToUObject());
}

template<>
FORCEINLINE UScriptStruct* CastFieldVariant<UScriptStruct>(FFieldVariant& FieldVariant)
{
	return ::Cast<UScriptStruct>(FieldVariant.ToUObject());
}

FORCEINLINE FString SafeNameToString(const FName& Value)
{
	return Value.IsValid() ? Value.ToString() : TEXT("<invalid-name>");
}

FName GetStructTypeName(FFieldVariant& Property);

}	// namespace DcPropertyUtils

