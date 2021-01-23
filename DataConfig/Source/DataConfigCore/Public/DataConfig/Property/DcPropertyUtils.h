#pragma once

#include "DataConfig/DcTypes.h"
#include "UObject/UnrealType.h"
#include "Templates/Function.h"

enum class EDcDataEntry : uint16;
struct FDcPropertyDatum;

namespace DcPropertyUtils
{
DATACONFIGCORE_API bool IsEffectiveProperty(FProperty* Property);
DATACONFIGCORE_API bool IsScalarProperty(FField* Property);
DATACONFIGCORE_API void VisitAllEffectivePropertyClass(TFunctionRef<void(FFieldClass*)> Visitor);

DATACONFIGCORE_API FProperty* NextEffectiveProperty(FProperty* Property);
DATACONFIGCORE_API FProperty* FirstEffectiveProperty(FProperty* Property);
DATACONFIGCORE_API FProperty* NextEffectivePropertyByName(FProperty* InProperty, const FName& Name);
DATACONFIGCORE_API FProperty* FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset);

DATACONFIGCORE_API FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue);

DATACONFIGCORE_API EDcDataEntry PropertyToDataEntry(const FFieldVariant& Field);
DATACONFIGCORE_API EDcDataEntry PropertyToDataEntry(FField* Property);

DATACONFIGCORE_API FString GetFormatPropertyTypeName(FField* Property);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(UScriptStruct* Struct);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(UClass* Class);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(const FFieldVariant& Field);

DATACONFIGCORE_API bool IsSubObjectProperty(FObjectProperty* ObjectProperty);
DATACONFIGCORE_API bool IsUnsignedProperty(FNumericProperty* NumericProperty);

DATACONFIGCORE_API FName GetStructTypeName(FFieldVariant& Property);
DATACONFIGCORE_API UScriptStruct* TryGetStructClass(FFieldVariant& FieldVariant);
DATACONFIGCORE_API UStruct* TryGetStruct(const FDcPropertyDatum& Datum);

FORCEINLINE FString SafeNameToString(const FName& Value)
{
	return Value.IsValid() ? Value.ToString() : TEXT("<invalid-name>");
}


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
template<> struct TPropertyTypeMap<FFieldPath> { using Type = FFieldPathProperty; };

template<> struct TPropertyTypeMap<FScriptDelegate> { using Type = FDelegateProperty; };
template<> struct TPropertyTypeMap<FMulticastScriptDelegate> { using Type = FMulticastInlineDelegateProperty; };

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
}	// namespace DcPropertyUtils

