#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _JSONDetails[] = {
	{ DcDJSON::ExpectWordButNotFound, TEXT("Expect word '{0}' but found '{1}' instead."), },
	{ DcDJSON::UnexpectedChar, TEXT("Unexpected char '{0}'"), },
	{ DcDJSON::UnexpectedToken, TEXT("Unexpected token"), },
	{ DcDJSON::UnclosedBlockComment, TEXT("Unclosed block comment"), },
	{ DcDJSON::UnclosedStringLiteral, TEXT("Unclosed string literal"), },
	{ DcDJSON::InvalidStringEscaping, TEXT("Invalid string escaping"), },
	{ DcDJSON::InvalidControlCharInString, TEXT("Invalid control character found in string"), },
	{ DcDJSON::ReadUnsignedWithNegativeNumber, TEXT("Reading unsigned with negative number"), },
	{ DcDJSON::ParseIntegerFailed, TEXT("Parse integer failed"), },
	{ DcDJSON::DuplicatedKey, TEXT("Duplicated key within object"), },
	{ DcDJSON::KeyMustBeString, TEXT("Object key must be a string"), },
	{ DcDJSON::ReadTypeMismatch, TEXT("Reading type mismatch, expecting '{0}' actual '{1}'"), },
	{ DcDJSON::ExpectComma, TEXT("Expect comma but not found"), },
	{ DcDJSON::ObjectKeyTooLong, TEXT("Object key string too long, UE4 FName has lengh limitation anyway"), },
};

FDcDiagnosticGroup DJSONDetails = {
	DcDJSON::Category,
	DcDimOf(_JSONDetails),
	_JSONDetails 
};

