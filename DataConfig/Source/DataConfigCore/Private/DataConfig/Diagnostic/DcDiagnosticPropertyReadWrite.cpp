#include "DataConfig/Diagnostic/DcDiagnosticPropertyReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _PropertyReadWriteDetails[] = {
	{ DPropertyReadWrite::InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ DPropertyReadWrite::InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ DPropertyReadWrite::PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
};

FDiagnosticGroup DPropertyReadWriteDetails = { DimOf(_PropertyReadWriteDetails), _PropertyReadWriteDetails };

} // namespace DataConfig


