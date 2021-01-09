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
};

FDcDiagnosticGroup Details = {
	DcDCommon::Category,
	DcDimOf(_CommonDetails),
	_CommonDetails
};

} // namespace DcDCommon
