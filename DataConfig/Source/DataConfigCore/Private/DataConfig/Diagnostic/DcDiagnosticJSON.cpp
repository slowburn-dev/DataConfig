#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _JSONDetails[] = {
	{ DcDJSON::UnexpectedEnd, TEXT("Unexpected End"), },
	{ DcDJSON::AlreadyEndedButExpect, TEXT("Input already ended but expect '{0}'"), },
	{ DcDJSON::ExpectWordButNotFound, TEXT("Expect word '{0}' but found '{1}' instead."), },
	{ DcDJSON::UnexpectedChar, TEXT("Unexpected char '{0}'"), },
	{ DcDJSON::UnexpectedToken, TEXT("Unexpected token '{0}'"), },
	{ DcDJSON::UnclosedBlockComment, TEXT("Unclosed block comment"), },
};

FDcDiagnosticGroup DJSONDetails = { DcDimOf(_JSONDetails), _JSONDetails };

