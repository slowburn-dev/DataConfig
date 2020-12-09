#pragma once

//	TODO namespace this and expose to Public
//		as user deserialize code obviously need all these
class FProperty;
class UStruct;

//	TODO eventually remove this method, because we support all property
bool IsEffectiveProperty(FProperty* Property);

bool IsScalarProperty(FField* Property);

size_t CountEffectiveProperties(UStruct* Struct);

FProperty* NextEffectiveProperty(FProperty* Property);
FProperty* FirstEffectiveProperty(FProperty* Property);
FProperty* NextPropertyByName(FProperty* InProperty, const FName& Name);

enum class EDcDataEntry : uint16;

EDcDataEntry PropertyToDataEntry(const FFieldVariant& Field);
EDcDataEntry PropertyToDataEntry(FField* Property);

FString GetFormatPropertyTypeName(FField* Property);
FString GetFormatPropertyTypeName(UScriptStruct* Struct);
FString GetFormatPropertyTypeName(UClass* Class);
FString GetFormatPropertyTypeName(const FFieldVariant& Field);

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

//	for read datum -> scalar
template<typename TProperty, typename TScalar>
void ReadPropertyValueConversion(FField* Property, void const* Ptr, TScalar* OutPtr)
{
	*OutPtr = (const TScalar&)(CastFieldChecked<TProperty>(Property)->GetPropertyValue(Ptr));
}

template<>
void ReadPropertyValueConversion<FBoolProperty, bool>(FField* Property, void const* Ptr, bool* OutPtr)
{
	*OutPtr = CastFieldChecked<FBoolProperty>(Property)->GetPropertyValue(Ptr);
}

template<>
void ReadPropertyValueConversion<FSoftObjectProperty, FSoftObjectPath>(FField* Property, void const* Ptr, FSoftObjectPath* OutPtr)
{
	*OutPtr = CastFieldChecked<FSoftObjectProperty>(Property)->GetPropertyValue(Ptr).GetUniqueID();
}

template<>
void ReadPropertyValueConversion<FSoftClassProperty, FSoftClassPath>(FField* Property, void const* Ptr, FSoftClassPath* OutPtr)
{
	static_assert(sizeof(FSoftClassPath) == sizeof(FSoftObjectPath), "should have same layout");
	*OutPtr = (const FSoftClassPath&)(CastFieldChecked<FSoftClassProperty>(Property)->GetPropertyValue(Ptr).GetUniqueID());
}

//	for writing scalar -> datum
template<typename TProperty, typename TScalar>
void WritePropertyValueConversion(FField* Property, void* Ptr, const TScalar& Value)
{
	CastFieldChecked<TProperty>(Property)->SetPropertyValue(Ptr, Value);
}

template<>
void WritePropertyValueConversion<FSoftObjectProperty, FSoftObjectPath>(FField* Property, void* Ptr, const FSoftObjectPath& Value)
{
	FSoftObjectPtr SoftPtr(Value);
	CastFieldChecked<FSoftObjectProperty>(Property)->SetPropertyValue(Ptr, SoftPtr);
}

template<>
void WritePropertyValueConversion<FSoftClassProperty, FSoftClassPath>(FField* Property, void* Ptr, const FSoftClassPath& Value)
{
	FSoftObjectPtr SoftPtr(Value);
	CastFieldChecked<FSoftClassProperty>(Property)->SetPropertyValue(Ptr, SoftPtr);
}

namespace DcPropertyHighlight
{
	enum class EFormatSeg
	{
		Normal,
		ParentIsContainer,
		Last,
	};

	void FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType);
	void FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, FProperty* Property);
	void FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, FProperty* Property);
	void FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, FMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue);;
	void FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, FArrayProperty* ArrayProperty, uint16 Index, bool bIsItem);
	void FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, FSetProperty* SetProperty, uint16 Index, bool bIsItem);

} // namespace DcPropertyHighlight




