#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

struct FDcDiagnosticDetail _DeserializeDetails[] = {
	{ DcDDeserialize::NoMatchingHandler, TEXT("No matching handler, Property: {0} {1}"), },
	{ DcDDeserialize::NotPrepared, TEXT("Deserialize isn't prepared, need to call 'Ctx.Prepare'"), },
	{ DcDDeserialize::DataEntryMismatch, TEXT("Deserialize type mismatch, Expect: {0}, Actual: {1}"), },
	{ DcDDeserialize::DataEntryMismatch2, TEXT("Deserialize type mismatch, Expect: {0}/{1}, Actual: {2}"), },
	{ DcDDeserialize::DataEntryMismatch3, TEXT("Deserialize type mismatch, Expect: {0}/{1}/{2}, Actual: {3}"), },
	{ DcDDeserialize::ExpectMetaType, TEXT("Expect '$type' but not found")},
	{ DcDDeserialize::PropertyMismatch, TEXT("Property type mismatch, Expect: {0}, Actual: {1}"), },
};

FDcDiagnosticGroup DDeserializeDetails = { DcDimOf(_DeserializeDetails), _DeserializeDetails };

