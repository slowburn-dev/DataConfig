#pragma once

#include "DataConfig/DcTypes.h"

struct FDcPropertyDatum;

namespace DcAutomationUtils
{

FDcResult TestReadDatumEqual(const FDcPropertyDatum& Lhs, const FDcPropertyDatum& Rhs);

}	// namespace DcAutomationUtils



