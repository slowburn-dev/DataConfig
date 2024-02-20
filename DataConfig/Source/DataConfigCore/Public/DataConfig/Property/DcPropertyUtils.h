#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "DataConfig/DcTypes.h"
#include "UObject/UnrealType.h"
#include "Templates/Function.h"
#include "UObject/TextProperty.h"
#include "Misc/EngineVersionComparison.h"

enum class EDcDataEntry : uint16;
struct FDcPropertyDatum;

namespace DcPropertyUtils
{

DATACONFIGCORE_API extern const FName DC_META_SKIP;

FORCEINLINE bool IsEffectiveProperty(FProperty* Property)
{
	check(Property);
#if DC_BUILD_DEBUG
	//	hook for globally ignore a property type
	//	this is for development only.
	//	For example if UE added a new property `FShinyProperty` we can start
	//	with ignoring it globally like this

	//	`return !Property->IsA<FShinyProperty>();`
	return true;
#else
	return true;
#endif
}

DATACONFIGCORE_API bool IsScalarProperty(FField* Property);
DATACONFIGCORE_API bool IsScalarArray(FField* Property);
DATACONFIGCORE_API bool IsScalarArray(FFieldVariant Property);
DATACONFIGCORE_API void VisitAllEffectivePropertyClass(TFunctionRef<void(FFieldClass*)> Visitor);

DATACONFIGCORE_API FProperty* NextEffectiveProperty(FProperty* Property);
DATACONFIGCORE_API FProperty* FirstEffectiveProperty(FProperty* Property);
DATACONFIGCORE_API FProperty* FindEffectivePropertyByName(UStruct* Struct, const FName& Name);

DATACONFIGCORE_API FProperty* FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset);
DATACONFIGCORE_API FDcResult FindEffectivePropertyByOffset(UStruct* Struct, size_t Offset, FProperty*& OutValue);

DATACONFIGCORE_API EDcDataEntry PropertyToDataEntry(const FFieldVariant& Field);
DATACONFIGCORE_API EDcDataEntry PropertyToDataEntry(FField* Property);

DATACONFIGCORE_API FString FormatArrayTypeName(FProperty* InnerProperty);
DATACONFIGCORE_API FString FormatSetTypeName(FProperty* InnerProperty);
DATACONFIGCORE_API FString FormatMapTypeName(FProperty* KeyProperty, FProperty* ValueProperty);
DATACONFIGCORE_API FString FormatOptionalTypeName(FProperty* ValueProperty);

DATACONFIGCORE_API FString GetFormatPropertyTypeName(FField* Property);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(UScriptStruct* Struct);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(UClass* Class);
DATACONFIGCORE_API FString GetFormatPropertyTypeName(const FFieldVariant& Field);

#if ENGINE_MAJOR_VERSION == 5
FORCEINLINE FString GetFormatPropertyTypeName(TObjectPtr<UClass> Ptr) { return GetFormatPropertyTypeName(Ptr.Get()); }
FORCEINLINE FString GetFormatPropertyTypeName(TObjectPtr<UScriptStruct> Ptr) { return GetFormatPropertyTypeName(Ptr.Get()); }
#endif

DATACONFIGCORE_API bool IsSubObjectProperty(FObjectProperty* ObjectProperty);
DATACONFIGCORE_API bool IsUnsignedProperty(FNumericProperty* NumericProperty);

DATACONFIGCORE_API FName GetStructTypeName(FFieldVariant& Property);
DATACONFIGCORE_API UScriptStruct* TryGetStructClass(FFieldVariant& FieldVariant);

DATACONFIGCORE_API UStruct* TryGetStruct(const FFieldVariant& FieldVariant);
DATACONFIGCORE_API UStruct* TryGetStruct(const FDcPropertyDatum& Datum);

DC_NODISCARD DATACONFIGCORE_API bool IsEnumAndTryUnwrapEnum(const FFieldVariant& Field, UEnum*& OutEnum, FNumericProperty*& OutNumeric);

DATACONFIGCORE_API bool HeuristicIsPointerInvalid(const void* Ptr);
DATACONFIGCORE_API FDcResult HeuristicVerifyPointer(const void* Ptr);

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
template<> struct TPropertyTypeMap<FScriptInterface> { using Type = FInterfaceProperty; };
template<> struct TPropertyTypeMap<FFieldPath> { using Type = FFieldPathProperty; };

template<> struct TPropertyTypeMap<FScriptDelegate> { using Type = FDelegateProperty; };
template<> struct TPropertyTypeMap<FMulticastScriptDelegate> { using Type = FMulticastInlineDelegateProperty; };

template<typename T>
struct TIsInPropertyMap
{
	template <typename C> static uint16 Test(typename TPropertyTypeMap<C>::Type*);
	template <typename C> static uint8 Test(...);

	enum { Value = sizeof(Test<T>(0)) - 1 };
};



template<typename T>
FORCEINLINE T* CastFieldVariant(const FFieldVariant& FieldVariant)
{
	static_assert(TIsDerivedFrom<T, FField>::Value, "expect TProperty to be a field");
	return ::CastField<T>(FieldVariant.ToField());
}

template<>
FORCEINLINE UClass* CastFieldVariant<UClass>(const FFieldVariant& FieldVariant)
{
	return ::Cast<UClass>(FieldVariant.ToUObject());
}

template<>
FORCEINLINE UScriptStruct* CastFieldVariant<UScriptStruct>(const FFieldVariant& FieldVariant)
{
	return ::Cast<UScriptStruct>(FieldVariant.ToUObject());
}

DATACONFIGCORE_API extern const FName DC_TRANSIENT_PROPERTY;

struct DATACONFIGCORE_API FDcPropertyBuilder
{
	FProperty* Property;

	template<typename TProperty>
	FORCEINLINE TProperty* As() { return CastFieldChecked<TProperty>(Property); }

	FORCEINLINE operator FProperty*() { return Property; }

	static FDcPropertyBuilder Make( FFieldClass* PropertyClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr );

	template<typename TProperty>
	static FDcPropertyBuilder Make( const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make(TProperty::StaticClass(), InName, InOuter); }

	FORCEINLINE static FDcPropertyBuilder Int8(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FInt8Property>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Int16(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FInt16Property>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Int(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FIntProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Int64(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FInt64Property>(InName, InOuter); }

	FORCEINLINE static FDcPropertyBuilder Byte(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FByteProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder UInt16(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FUInt16Property>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder UInt32(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FUInt32Property>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder UInt64(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FUInt64Property>(InName, InOuter); }

	FORCEINLINE static FDcPropertyBuilder Float(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FFloatProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Double(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FDoubleProperty>(InName, InOuter); }

	FORCEINLINE static FDcPropertyBuilder Name(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FNameProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Str(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FStrProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder Text(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FTextProperty>(InName, InOuter); }
	FORCEINLINE static FDcPropertyBuilder FieldPath(const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr) { return Make<FFieldPathProperty>(InName, InOuter); }

	static FDcPropertyBuilder Object(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Class(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Struct(UScriptStruct* InStruct, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder SoftObject(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder SoftClass(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder LazyObject(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Delegate(UFunction* InFunction, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder MulticastInlineDelegate(UDelegateFunction* InFunction, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder MulticastSparseDelegate(USparseDelegateFunction* InFunction, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);

	static FDcPropertyBuilder Enum(UEnum* InEnum, FProperty* InUnderlying, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Enum(UEnum* InEnum, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Byte(UEnum* InEnum, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Bool(uint32 InSize = sizeof(bool), const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);

	static FDcPropertyBuilder Array(FProperty* InInner, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Set(FProperty* InInner, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder Map(FProperty* InKey, FProperty* InValue, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3
	static FDcPropertyBuilder ObjectPtr(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
	static FDcPropertyBuilder ClassPtr(UClass* InClass, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
#endif //ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 3

#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	static FDcPropertyBuilder Optional(FProperty* InInner, const FName InName = DC_TRANSIENT_PROPERTY, FFieldVariant InOuter = nullptr);
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

	FDcPropertyBuilder& ArrayDim(int InArrayDim);

	void Link();

	FORCEINLINE TUniquePtr<FProperty> LinkOnScope()
	{
		Link();
		return TUniquePtr<FProperty>(Property);
	}

	template<typename TProperty>
	TProperty* LinkAndCast()
	{
		Link();
		return CastFieldChecked<TProperty>(Property);
	}

	template<typename TProperty>
	TUniquePtr<TProperty> LinkAndCastOnScope()
	{
		Link();
		return TUniquePtr<TProperty>(CastFieldChecked<TProperty>(Property));
	}
};

}	// namespace DcPropertyUtils

