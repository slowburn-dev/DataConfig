#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDJSON
{

static FDcDiagnosticDetail _JSONDetails[] = {
	{ ExpectWordButNotFound, TEXT("Expect word '{0}' but found '{1}' instead."), },
	{ ExpectWordButEOF, TEXT("Expect word '{0}' but reaching end of input."), },
	{ UnexpectedChar, TEXT("Unexpected char '{0}'"), },
	{ UnexpectedToken, TEXT("Unexpected token"), },
	{ UnexpectedToken, TEXT("Unexpected EOF"), },
	{ UnclosedBlockComment, TEXT("Unclosed block comment"), },
	{ UnclosedStringLiteral, TEXT("Unclosed string literal"), },
	{ InvalidStringEscaping, TEXT("Invalid string escaping"), },
	{ InvalidControlCharInString, TEXT("Invalid control character found in string"), },
	{ ReadUnsignedWithNegativeNumber, TEXT("Reading unsigned with negative number"), },
	{ ParseIntegerFailed, TEXT("Parse integer failed"), },
	{ DuplicatedKey, TEXT("Duplicated key within object: {0}"), },
	{ KeyMustBeString, TEXT("Object key must be a string"), },
	{ ReadTypeMismatch, TEXT("Reading type mismatch, expecting '{0}' actual '{1}'"), },
	{ ExpectComma, TEXT("Expect ',' but not found"), },
	{ ExpectStateInProgress, TEXT("Expect internal state to be 'InProgress', Actual: {0}"), },
	{ ExpectStateUninitializedOrFinished, TEXT("Expect internal state to be 'Uninitialized' or 'Finished', Actual: {0}"), },
	{ UnexpectedTrailingToken, TEXT("Expect ending but found trailing tokens, Actual: {0}"), },

	//	Number
	{ NumberInvalidChar, TEXT("Invalid char in number: '{0}'"), },
	{ NumberExpectDigitAfterMinus, TEXT("Number expect digit after '-' but found: '{0}'"), },
	{ NumberExpectDigitAfterDot, TEXT("Number expect digit after '.' but found: '{0}'"), },
	{ NumberExpectSignDigitAfterExp, TEXT("Number expect '-', '+' or digit after '.' but found: '{0}'"), },
	{ NumberExpectDigitAfterExpSign, TEXT("Number expect digit after '.+/-' but found: '{0}'"), },

	//	End
	{ EndUnclosedArray, TEXT("End with unclosed array"), },
	{ EndUnclosedObject, TEXT("End with unclosed object"), },

	//	Writer
	{ ExpectStringAtObjectKey, TEXT("Expect only string at object key"), },
	{ UnexpectedArrayEnd, TEXT("Unexpected write array end"), },
	{ UnexpectedObjectEnd, TEXT("Unexpected write object end"), },

};

FDcDiagnosticGroup Details = {
	Category,
	DcDimOf(_JSONDetails),
	_JSONDetails
};

} // namespace DcDJSON
