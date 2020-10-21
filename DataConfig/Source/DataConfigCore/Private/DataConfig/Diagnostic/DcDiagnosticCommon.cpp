#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _CommonDetails[] = {
	{ DCommon::NotImplemented, TEXT("Calling into methods that are not implemented") },
	{ DCommon::Unreachable, TEXT("Calling into unreachable code") },
	{ DCommon::Unexpected1, TEXT("Unexpected: {0}") },
	{ DCommon::PlaceHoldError, TEXT("<<<placeholder error>>>") },
};

FDcDiagnosticGroup DCommonDetails = { DcDimOf(_CommonDetails), _CommonDetails };

