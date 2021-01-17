#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

struct FDcPropertyReader;
struct FDcPropertyWriter;

namespace DcExtra {

///	Read/Write arbitrary nested inner field by a JSONPath/XMLPath like string

DATACONFIGEXTRA_API FDcResult TraverseReaderByPath(FDcPropertyReader* Reader, const FString& Path);

template<typename T>
T GetDatumPropertyByPath(const FDcPropertyDatum& Datum, const FString& Path); 


} // namespace DcExtra
