#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _CommonDetails[] = {
	{ DCommon::NotImplemented, TEXT("Calling into methods that are not implemented.") },
	{ DCommon::Fun, TEXT("Try Args {0} {1} {2}") },
};

FDiagnosticGroup DCommonDetails = { DimOf(_CommonDetails), _CommonDetails };

} // namespace DataConfig
