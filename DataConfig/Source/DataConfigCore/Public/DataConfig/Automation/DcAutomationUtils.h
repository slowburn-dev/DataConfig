#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DcAutomationUtils
{

DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum);

DATACONFIGCORE_API FDcResult DumpToLog(FDcPropertyDatum Datum);

}	// namespace DcAutomationUtils



