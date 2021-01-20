#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

#include "DcPropertyPathAccess.generated.h"

struct FDcPropertyReader;
struct FDcPropertyWriter;

namespace DcExtra {

///	Read/Write arbitrary nested inner field by a JSONPath/XMLPath like string

DATACONFIGEXTRA_API FDcResult TraverseReaderByPath(FDcPropertyReader* Reader, const FString& Path);

template<typename T>
T GetDatumPropertyByPath(const FDcPropertyDatum& Datum, const FString& Path); 

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
struct FDcExtraTestStructNestRoot
{
	GENERATED_BODY()

	UPROPERTY() FDcExtraTestStructNestMiddle Middle;
};

