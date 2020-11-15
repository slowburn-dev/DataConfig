#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

static FDcDiagnosticDetail _ReadWriteDetails[] = {
	{ DcDReadWrite::InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ DcDReadWrite::InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ DcDReadWrite::InvalidStateWithExpect2, TEXT("Invalid internal state. Expect {0} {1}, Actual {2}."), },
	{ DcDReadWrite::DataTypeMismatch, TEXT("Datatype mismatch. Expected {0}, Actual {1}") },
	{ DcDReadWrite::DataTypeMismatch2, TEXT("Datatype mismatch. Expected {0}, {1}, Actual {2}") },
	{ DcDReadWrite::PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
	{ DcDReadWrite::AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ DcDReadWrite::CantFindPropertyByName, TEXT("Can't find property by name. Name {0}") },
	{ DcDReadWrite::WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property {0} {1}") },
	{ DcDReadWrite::StructNameMismatch, TEXT("Struct name mismatch, Expect {0}, Actual {1}") },
	//	putback reader
	{ DcDReadWrite::CantUsePutbackValue, TEXT("Cannot use putback value in {0}") },
};

FDcDiagnosticGroup DPropertyReadWriteDetails = { DcDimOf(_ReadWriteDetails), _ReadWriteDetails };


