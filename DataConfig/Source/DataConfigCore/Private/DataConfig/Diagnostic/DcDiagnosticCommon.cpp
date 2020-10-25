#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _CommonDetails[] = {
	{ DcDCommon::Unhandled, TEXT("Unhandled error") },
	{ DcDCommon::NotImplemented, TEXT("Calling into methods that are not implemented") },
	{ DcDCommon::Unreachable, TEXT("Calling into unreachable code") },
	{ DcDCommon::Unexpected1, TEXT("Unexpected: {0}") },
	{ DcDCommon::PlaceHoldError, TEXT("<<<placeholder error>>>") },
};

FDcDiagnosticGroup DCommonDetails = { DcDimOf(_CommonDetails), _CommonDetails };

