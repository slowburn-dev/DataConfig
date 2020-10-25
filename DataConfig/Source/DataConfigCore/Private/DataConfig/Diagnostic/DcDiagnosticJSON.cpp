#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _JSONDetails[] = {
	{ DcDJSON::UnexpectedEnd, TEXT("Unexpected End"), },
	{ DcDJSON::AlreadyEndedButExpect, TEXT("Input already ended but expect {0}"), },
	{ DcDJSON::ExpectWordButNotFound, TEXT("Expect word but not found {0}"), },
	{ DcDJSON::ExpectCharButNotFound, TEXT("Expect char but not found {0}"), },
	{ DcDJSON::UnexpectedChar1, TEXT("Unexpected char {0}"), },
	{ DcDJSON::UnexpectedToken, TEXT("Expect token {0} but found {1}"), },
};

FDcDiagnosticGroup DJSONDetails = { DcDimOf(_JSONDetails), _JSONDetails };

