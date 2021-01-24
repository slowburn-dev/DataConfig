#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
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
T GetDatumPropertyByPath(const FDcPropertyDatum& RootDatum, const FString& Path)
{
	using TProperty = typename DcPropertyUtils::TPropertyTypeMap<T>::Type;

	FDcPropertyDatum ResultDatum;
	FDcResult Ret = GetDatumPropertyByPath(RootDatum, Path, ResultDatum);
	if (!Ret.Ok())
		return T{};

	TProperty* Property = ResultDatum.CastField<TProperty>();
	if (!Property)
		return T{};

	return Property->GetPropertyValue(ResultDatum.DataPtr);
}

} // namespace DcExtra


USTRUCT()
struct FDcExtraTestStructNestInnerMost
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;

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

