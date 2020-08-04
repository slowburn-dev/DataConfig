#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

static FDiagnosticDetail _ReadWriteDetails[] = {
	{ DReadWrite::InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ DReadWrite::InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ DReadWrite::InvalidStateWithExpect2, TEXT("Invalid internal state. Expect {0} {1}, Actual {2}."), },
	{ DReadWrite::DataTypeMismatch, TEXT("Datatype mismatch. Expected {0}, Actual {1}") },
	{ DReadWrite::DataTypeMismatch2, TEXT("Datatype mismatch. Expected {0}, {1}, Actual {2}") },
	{ DReadWrite::PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
	{ DReadWrite::AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ DReadWrite::CantFindPropertyByName, TEXT("Can't find property by name. Name {0}") },
	{ DReadWrite::WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property {0} {1}") },
	{ DReadWrite::StructNameMismatch, TEXT("Struct name mismatch, Expect {0}, Actual {1}") },
};

FDiagnosticGroup DPropertyReadWriteDetails = { DimOf(_ReadWriteDetails), _ReadWriteDetails };

} // namespace DataConfig


