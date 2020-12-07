#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"

static FDcDiagnosticDetail _ReadWriteDetails[] = {
	{ DcDReadWrite::InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ DcDReadWrite::InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ DcDReadWrite::InvalidStateWithExpect2, TEXT("Invalid internal state. Expect {0} {1}, Actual {2}."), },
	{ DcDReadWrite::DataTypeMismatch, TEXT("Datatype mismatch. Expected {0}, Actual {1}") },
	{ DcDReadWrite::DataTypeMismatch2, TEXT("Datatype mismatch. Expected {0}, {1}, Actual {2}") },
	{ DcDReadWrite::PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
	{ DcDReadWrite::AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ DcDReadWrite::CantFindPropertyByName, TEXT("Can't find property by name. Name '{0}'") },
	{ DcDReadWrite::WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property {0} {1}") },
	{ DcDReadWrite::StructNameMismatch, TEXT("Struct name mismatch, Expect {0}, Actual {1}") },
	{ DcDReadWrite::UInt64EnumNotSupported, TEXT("Doesn't support uint64 backed enum atm") },
	{ DcDReadWrite::EnumNameMismatch, TEXT("Enum name mismatch, Expect {0}, Actual {1}") },
	{ DcDReadWrite::EnumNameNotFound, TEXT("Enum name not found. Enum {0}, Actual {1}") },
	{ DcDReadWrite::EnumValueInvalid, TEXT("Enum value isn't valid for this enum. Enum {0}, Actual {1}") },
	{ DcDReadWrite::WriteBlobOverrun, TEXT("Writing blob overrun, Expected len {0}, Actual len {1}") },
	//	putback reader
	{ DcDReadWrite::CantUsePutbackValue, TEXT("Cannot use putback value in {0}") },
	//	pipe visitor
	{ DcDReadWrite::PipeReadWriteMismatch, TEXT("Pipe visit read write mismatch. Actual {0}") },
};

FDcDiagnosticGroup DPropertyReadWriteDetails = { DcDimOf(_ReadWriteDetails), _ReadWriteDetails };

