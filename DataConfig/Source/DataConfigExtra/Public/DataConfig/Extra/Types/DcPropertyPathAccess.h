#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyDatum.h"

#include "DcPropertyPathAccess.generated.h"

struct FDcPropertyReader;
struct FDcPropertyWriter;

namespace DcExtra {

///	Access nested properties by a path like `Foo.Bar.Baz` similar to module `PropertyPathHelpers`

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
};

UCLASS()
class UDcExtraTestClassOuter : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY() FDcExtraTestStructNestOuter StructRoot;
};

