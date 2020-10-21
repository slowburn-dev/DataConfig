#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

struct FDcDiagnosticDetail _DeserializeDetails[] = {
	{ DDeserialize::NoMatchingHandler, TEXT("No matching handler, Property: {0} {1}"), },
	{ DDeserialize::DataEntryMismatch, TEXT("Deserialize type mismatch, Expect: {0}, Actual: {1}"), },
	{ DDeserialize::DataEntryMismatch2, TEXT("Deserialize type mismatch, Expect: {0}/{1}, Actual: {2}"), },
	{ DDeserialize::DataEntryMismatch3, TEXT("Deserialize type mismatch, Expect: {0}/{1}/{2}, Actual: {3}"), },
};

FDcDiagnosticGroup DDeserializeDetails = { DcDimOf(_DeserializeDetails), _DeserializeDetails };

