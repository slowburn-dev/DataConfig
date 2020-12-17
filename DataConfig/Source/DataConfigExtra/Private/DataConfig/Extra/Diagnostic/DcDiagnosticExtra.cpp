#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDExtra
{

static FDcDiagnosticDetail _ExtraDetails[] = {
	{ InvalidBase64String, TEXT("Invalid Base64 string failed to convert to blob"), },
};

 FDcDiagnosticGroup Details = {
	DcDExtra::Category,
	DcDimOf(_ExtraDetails),
	_ExtraDetails
};


} // namespace DcDExtra
