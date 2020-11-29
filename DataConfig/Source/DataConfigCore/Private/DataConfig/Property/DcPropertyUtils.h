#pragma once

class UProperty;
class UStruct;

bool IsEffectiveProperty(UProperty* Property);

bool IsScalarProperty(UField* Property);

size_t CountEffectiveProperties(UStruct* Struct);

UProperty* NextEffectiveProperty(UProperty* Property);
UProperty* FirstEffectiveProperty(UProperty* Property);
UProperty* NextPropertyByName(UProperty* InProperty, const FName& Name);

enum class EDcDataEntry;
EDcDataEntry PropertyToDataEntry(UField* Property);

FString GetFormatPropertyTypeName(UField* Property);

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
template<> struct TPropertyTypeMap<FText> { using Type = UTextProperty; };

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

template<> struct TPropertyTypeMap<UClass*> { using Type = UClassProperty; };
template<> struct TPropertyTypeMap<UObject*> { using Type = UObjectProperty; };

template<> struct TPropertyTypeMap<FWeakObjectPtr> { using Type = UWeakObjectProperty; };
template<> struct TPropertyTypeMap<FLazyObjectPtr> { using Type = ULazyObjectProperty; };
template<> struct TPropertyTypeMap<FSoftObjectPath> { using Type = USoftObjectProperty; };
template<> struct TPropertyTypeMap<FSoftClassPath> { using Type = USoftClassProperty; };

static_assert(TIsSame<TPropertyTypeMap<int32>::Type, UIntProperty>::Value, "yes");

template<typename TProperty, typename TScalar>
void WritePropertyValueConversion(UField* Property, void const* Ptr, TScalar* OutPtr)
{
	*OutPtr = (const TScalar&)(CastChecked<TProperty>(Property)->GetPropertyValue(Ptr));
}


namespace DcPropertyHighlight
{
	enum class EFormatSeg { Normal, Last, };

	void FormatNil(TArray<FString>& OutSegments, EFormatSeg SegType);
	void FormatClass(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& ObjectName, UClass* Class, UProperty* Property);
	void FormatStruct(TArray<FString>& OutSegments, EFormatSeg SegType, const FName& StructName, UScriptStruct* StructClass, UProperty* Property);
	void FormatMap(TArray<FString>& OutSegments, EFormatSeg SegType, UMapProperty* MapProperty, uint16 Index, bool bIsKeyOrValue);;
	void FormatArray(TArray<FString>& OutSegments, EFormatSeg SegType, UArrayProperty* ArrayProperty, uint16 Index, bool bIsItem);
	void FormatSet(TArray<FString>& OutSegments, EFormatSeg SegType, USetProperty* SetProperty, uint16 Index, bool bIsItem);

} // namespace DcPropertyHighlight



