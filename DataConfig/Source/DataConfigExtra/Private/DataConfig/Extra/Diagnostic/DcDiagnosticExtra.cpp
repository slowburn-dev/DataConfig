#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDExtra
{

static FDcDiagnosticDetail _ExtraDetails[] = {
	{ InvalidBase64String, TEXT("Invalid Base64 string failed to convert to blob"), },
	{ ExpectClassExpand, TEXT("Expect read class control to be 'ExpandObject'. Remember to force expand from Config"), },
};

 FDcDiagnosticGroup Details = {
	DcDExtra::Category,
	DcDimOf(_ExtraDetails),
	_ExtraDetails
};


} // namespace DcDExtra
