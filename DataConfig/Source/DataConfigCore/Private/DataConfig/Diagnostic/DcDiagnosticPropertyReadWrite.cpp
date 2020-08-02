#include "DataConfig/Diagnostic/DcDiagnosticPropertyReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _PropertyReadWriteDetails[] = {
	{ DPropertyReadWrite::InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ DPropertyReadWrite::InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ DPropertyReadWrite::InvalidStateWithExpect2, TEXT("Invalid internal state. Expect {0} {1}, Actual {2}."), },
	{ DPropertyReadWrite::DataTypeMismatch, TEXT("Datatype mismatch. Expected {0}, Actual {1}") },
	{ DPropertyReadWrite::DataTypeMismatch2, TEXT("Datatype mismatch. Expected {0}, {1}, Actual {2}") },
	{ DPropertyReadWrite::PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
	{ DPropertyReadWrite::AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ DPropertyReadWrite::CantFindPropertyByName, TEXT("Can't find property by name. Name {0}") },
	{ DPropertyReadWrite::WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property {0} {1}") },
};

FDiagnosticGroup DPropertyReadWriteDetails = { DimOf(_PropertyReadWriteDetails), _PropertyReadWriteDetails };

} // namespace DataConfig


