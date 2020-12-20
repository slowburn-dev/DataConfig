#pragma once

#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

namespace DcAutomationUtils
{

//	TODO figureout why this DLLEXPORT can not be removed
DATACONFIGCORE_API FDcResult TestReadDatumEqual(const FDcPropertyDatum& LhsDatum, const FDcPropertyDatum& RhsDatum);

}	// namespace DcAutomationUtils



