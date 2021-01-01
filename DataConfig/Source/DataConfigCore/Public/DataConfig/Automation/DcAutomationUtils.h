#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DcAutomationUtils
{

DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum);

DATACONFIGCORE_API FDcResult DumpToLog(FDcPropertyDatum Datum);

DATACONFIGCORE_API void AmendMetaData(UField* Field, const FName& MetaKey, const TCHAR* MetaValue);
DATACONFIGCORE_API void AmendMetaData(UStruct* Struct, const FName& FieldName, const FName& MetaKey, const TCHAR* MetaValue);

}	// namespace DcAutomationUtils



