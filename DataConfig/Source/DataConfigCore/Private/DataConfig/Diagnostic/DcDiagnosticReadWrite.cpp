#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDReadWrite
{

static FDcDiagnosticDetail _ReadWriteDetails[] = {
	{ InvalidStateNoExpect, TEXT("Invalid internal state. Actual {0}."), },
	{ InvalidStateWithExpect, TEXT("Invalid internal state. Expect {0}, Actual {1}."), },
	{ InvalidStateWithExpect2, TEXT("Invalid internal state. Expect {0} {1}, Actual {2}."), },
	{ DataTypeMismatch, TEXT("Datatype mismatch. Expected {0}, Actual {1}") },
	{ DataTypeMismatch2, TEXT("Datatype mismatch. Expected {0}, {1}, Actual {2}") },
	{ PropertyMismatch, TEXT("Unexpected Property type. Expected type {0}, Property {0} {1}") },
	{ AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ CantFindPropertyByName, TEXT("Can't find property by name. Name '{0}'") },
	{ WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property {0} {1}") },
	{ StructNameMismatch, TEXT("Struct name mismatch, Expect {0}, Actual {1}") },
	{ ClassNameMismatch, TEXT("Class name mismatch, Expect {0}, Actual {1}") },
	{ UInt64EnumNotSupported, TEXT("Doesn't support uint64 backed enum atm") },
	{ EnumNameMismatch, TEXT("Enum name mismatch, Expect {0}, Actual {1}") },
	{ EnumNameNotFound, TEXT("Enum name not found. Enum {0}, Actual {1}") },
	{ EnumValueInvalid, TEXT("Enum value isn't valid for this enum. Enum {0}, Actual {1}") },
	{ WriteBlobOverrun, TEXT("Writing blob overrun, Expected len {0}, Actual len {1}") },
	//	putback reader
	{ CantUsePutbackValue, TEXT("Cannot use putback value in {0}") },
	//	pipe visitor
	{ PipeReadWriteMismatch, TEXT("Pipe visit read write mismatch. Actual {0}") },
};

FDcDiagnosticGroup Details = {
	DcDReadWrite::Category,
	DcDimOf(_ReadWriteDetails),
	_ReadWriteDetails
};

} // namespace DPropertyReadWrite
