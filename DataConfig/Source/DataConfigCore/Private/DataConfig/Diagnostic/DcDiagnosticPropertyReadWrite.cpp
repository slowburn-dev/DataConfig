#include "DataConfig/Diagnostic/DcDiagnosticPropertyReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _PropertyReadWriteDetails[] = {
	{ DPropertyReadWrite::InvalidState, TEXT("Invalid Internal State, Expect {0} Actual {1}"), },
};

FDiagnosticGroup DPropertyReadWriteDetails = { DimOf(_PropertyReadWriteDetails), _PropertyReadWriteDetails };

} // namespace DataConfig


