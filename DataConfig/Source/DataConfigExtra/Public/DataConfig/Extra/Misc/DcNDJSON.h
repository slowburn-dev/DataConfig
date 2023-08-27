#pragma once

/// NDJSON examples
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyUtils.h"

namespace DcExtra
{
    
DATACONFIGEXTRA_API FDcResult LoadNDJSON(const TCHAR* Str, FDcPropertyDatum Datum);

template<typename TStruct>
DATACONFIGEXTRA_API FDcResult LoadNDJSON(const TCHAR* Str, TArray<TStruct>& Arr)
{
    using namespace DcPropertyUtils;
    auto ArrProp = FDcPropertyBuilder::Array(
        FDcPropertyBuilder::Struct(TBaseStructure<TStruct>::Get())
    ).LinkOnScope();

    return LoadNDJSON(Str, FDcPropertyDatum(ArrProp.Get(), &Arr));
}

DATACONFIGEXTRA_API FDcResult SaveNDJSON(FDcPropertyDatum Datum, FString& OutStr);

template<typename TStruct>
DATACONFIGEXTRA_API FDcResult SaveNDJSON(const TArray<TStruct>& Arr, FString& OutStr)
{
    using namespace DcPropertyUtils;
    auto ArrProp = FDcPropertyBuilder::Array(
        FDcPropertyBuilder::Struct(TBaseStructure<TStruct>::Get())
    ).LinkOnScope();

    return SaveNDJSON(FDcPropertyDatum(ArrProp.Get(), (void*)&Arr), OutStr);
}

} // namespace DcExtra

