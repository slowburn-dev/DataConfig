#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

struct FDiagnosticDetail _DeserializeDetails[] = {
	{ DDeserialize::NoMatchingHandler, TEXT("No matching handler, Property: {0} {1}"), },
	{ DDeserialize::DataEntryMismatch, TEXT("Deserialize type mismatch, Expect: {0}, Actual: {1}"), },
	{ DDeserialize::DataEntryMismatch2, TEXT("Deserialize type mismatch, Expect: {0}/{1}, Actual: {2}"), },
};

FDiagnosticGroup DDeserializeDetails = { DimOf(_DeserializeDetails), _DeserializeDetails };


} // namespace DataConfig






