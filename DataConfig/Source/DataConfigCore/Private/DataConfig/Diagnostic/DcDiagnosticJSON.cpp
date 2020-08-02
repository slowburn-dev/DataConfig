#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _JSONDetails[] = {
	{ DJSON::UnexpectedEnd, TEXT("Unexpected End"), },
	{ DJSON::AlreadyEndedButExpect, TEXT("Input already ended but expect {0}"), },
	{ DJSON::ExpectWordButNotFound, TEXT("Expect word but not found {0}"), },
	{ DJSON::ExpectCharButNotFound, TEXT("Expect char but not found {0}"), },
	{ DJSON::UnexpectedChar1, TEXT("Unexpected char {0}"), },
};

FDiagnosticGroup DJSONDetails = { DimOf(_JSONDetails), _JSONDetails };

} // namespace DataConfig







