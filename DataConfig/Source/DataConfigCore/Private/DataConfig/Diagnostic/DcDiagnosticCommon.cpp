#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _CommonDetails[] = {
	{ DCommon::NotImplemented, TEXT("Calling into methods that are not implemented") },
	{ DCommon::Unreachable, TEXT("Calling into unreachable code") },
	{ DCommon::Unexpected1, TEXT("Unexpected: {0}") },
	{ DCommon::PlaceHoldError, TEXT("<<<placeholder error>>>") },
};

FDiagnosticGroup DCommonDetails = { DimOf(_CommonDetails), _CommonDetails };

} // namespace DataConfig
