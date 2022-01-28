#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDCommon
{

static FDcDiagnosticDetail _CommonDetails[] = {
	{ Unhandled, TEXT("Unhandled error, stack:\n{0}") },
	{ NotImplemented, TEXT("Calling into methods that are not implemented") },
	{ Unreachable, TEXT("Calling into unreachable code") },
	{ Unexpected1, TEXT("Unexpected: '{0}'") },
	{ PlaceHoldError, TEXT("<<<placeholder error>>>") },

	{ StaleDelegate, TEXT("Stale delegate") },
	{ StaleDelegateWithName, TEXT("Stale delegate: {0}") },

	{ CustomMessage, TEXT("Custom Diagnostic Message: {0}") },
};

FDcDiagnosticGroup Details = {
	Category,
	DcDimOf(_CommonDetails),
	_CommonDetails
};

} // namespace DcDCommon
