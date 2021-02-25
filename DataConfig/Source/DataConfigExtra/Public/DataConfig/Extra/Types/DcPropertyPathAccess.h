#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyDatum.h"

#include "DcPropertyPathAccess.generated.h"

struct FDcPropertyReader;
struct FDcPropertyWriter;

namespace DcExtra {

///	Access nested properties by a path like `Foo.Bar.Baz` similar to module `PropertyPathHelpers` with additional features
///	1. Support non UObject roots.
/// 2. Support access TArray elements like `Foo.2.Bar`
/// 3. Support access TMap elements like `Foo.MapKey.Bar`

DATACONFIGEXTRA_API FDcResult TraverseReaderByPath(FDcPropertyReader* Reader, const FString& Path);
DATACONFIGEXTRA_API FDcResult GetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path, FDcPropertyDatum& OutDatum);

template<typename T>
typename TEnableIf<DcTypeUtils::TIsUClass<T>::Value, T*>::Type
GetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path)
{
	FDcPropertyDatum ResultDatum;
	FDcResult Ret = GetDatumPropertyByPath(RootDatum, Path, ResultDatum);
	if (!Ret.Ok())
		return nullptr;

	FObjectProperty* Property = ResultDatum.CastField<FObjectProperty>();
	if (!Property)
		return nullptr;

	if (Property->PropertyClass != T::StaticClass())
		return nullptr;

	return (T*)Property->GetObjectPropertyValue(ResultDatum.DataPtr);
}

template<typename T>
typename TEnableIf<DcTypeUtils::TIsUStruct<T>::Value, T*>::Type
GetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path)
{
	FDcPropertyDatum ResultDatum;
	FDcResult Ret = GetDatumPropertyByPath(RootDatum, Path, ResultDatum);
	if (!Ret.Ok())
		return nullptr;

	FStructProperty* Property = ResultDatum.CastField<FStructProperty>();
	if (!Property)
		return nullptr;

	if (Property->Struct != T::StaticStruct())
		return nullptr;

	return (T*)(ResultDatum.DataPtr);
}
	
template<typename T>
typename TEnableIf<DcPropertyUtils::TIsInPropertyMap<T>::Value, T*>::Type
GetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<T>::Type;

	FDcPropertyDatum ResultDatum;
	FDcResult Ret = GetDatumPropertyByPath(RootDatum, Path, ResultDatum);
	if (!Ret.Ok())
		return nullptr;

	TProperty* Property = ResultDatum.CastField<TProperty>();
	if (!Property)
		return nullptr;

	return Property->GetPropertyValuePtr(ResultDatum.DataPtr);
}

template<typename T>
bool SetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path, const T& Value)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<T>::Type;

	FDcPropertyDatum ResultDatum;
	FDcResult Ret = GetDatumPropertyByPath(RootDatum, Path, ResultDatum);
	if (!Ret.Ok())
		return false;

	TProperty* Property = ResultDatum.CastField<TProperty>();
	if (!Property)
		return false;

	Property->SetPropertyValue(ResultDatum.DataPtr, Value);
	return true;
}

} // namespace DcExtra


USTRUCT()
struct FDcExtraTestStructNestInnerMost
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
	UPROPERTY() class UDcExtraTestClassOuter* ObjField;

};

USTRUCT()
struct FDcExtraTestStructNestMiddle
{
	GENERATED_BODY()

	UPROPERTY() FDcExtraTestStructNestInnerMost InnerMost;

};

USTRUCT()
struct FDcExtraTestStructNestOuter
{
	GENERATED_BODY()

	UPROPERTY() FDcExtraTestStructNestMiddle Middle;
	UPROPERTY() TArray<FDcExtraTestStructNestInnerMost> Arr;
	UPROPERTY() TMap<FName, FDcExtraTestStructNestInnerMost> NameMap;
};

UCLASS()
class UDcExtraTestClassOuter : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY() FDcExtraTestStructNestOuter StructRoot;
};

