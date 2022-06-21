#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDReadWrite
{

static FDcDiagnosticDetail _ReadWriteDetails[] = {
	{ InvalidStateNoExpect, TEXT("Invalid internal state. Actual '{0}'."), },
	{ InvalidStateWithExpect, TEXT("Invalid internal state. Expect '{0}', Actual '{1}'."), },
	{ InvalidStateWithExpect2, TEXT("Invalid internal state. Expect '{0}' '{1}', Actual '{2}'."), },
	{ DataTypeMismatch, TEXT("Datatype mismatch. Expected '{0}', Actual '{1}'") },
	{ DataTypeMismatch2, TEXT("Datatype mismatch. Expected '{0}', '{1}', Actual '{2}'") },
	{ DataTypeMismatchNoExpect, TEXT("Datatype mismatch. Actual '{0}'") },
	{ DataTypeMismatchNoCoercion, TEXT("Datatype mismatch and no valid coercion. Expected '{0}', Actual '{1}'") },
	{ PropertyMismatch, TEXT("Unexpected Property type. Expected type '{0}', Property '{1}' '{2}'") },
	{ PropertyMismatch2, TEXT("Unexpected Property type. Expected type '{0}' / '{1}', Property '{2}' '{3}'") },
	{ AlreadyEnded, TEXT("Invalid access after already ended.") },
	{ CantFindPropertyByName, TEXT("Can't find property by name. Name '{0}'") },
	{ WriteClassInlineNotCreated, TEXT("Expand class write need the object to already exist. Property '{0}' '{1}'") },
	{ StructNameMismatch, TEXT("Struct name mismatch, Expect '{0}', Actual '{1}'") },
	{ ClassNameMismatch, TEXT("Class name mismatch, Expect '{0}', Actual '{1}'") },
	{ EnumNameMismatch, TEXT("Enum name mismatch, Expect '{0}', Actual '{1}'") },
	{ EnumNameNotFound, TEXT("Enum name not found. Enum '{0}', Actual '{1}'") },
	{ EnumSignMismatch, TEXT("Enum sign mismatch. Expect unsigned: '{0}', Actual unsigned: '{1}'") },
	{ EnumMissingEnum, TEXT("Enum field missing enum, Property '{0}, '{1}") },
	{ EnumValueInvalid, TEXT("Enum value isn't valid for this enum. Enum '{0}', Actual '{1}'") },
	{ WriteBlobOverrun, TEXT("Writing blob overrun, Expected len '{0}', Actual len '{1}'") },
	{ FindPropertyByOffsetFailed, TEXT("Failed to find property by offset, Class: '{0}', Offset: '{1}'") },
	{ DataTypeUnequal, TEXT("Data entry value unequal, Type: '{0}'") },
	{ DataTypeUnequalLhsRhs, TEXT("Data entry value unequal, Type: '{0}', Lhs: '{1}', Rhs: '{2}'") },
	{ ExpectFieldButFoundUObject, TEXT("Expect FFieldVariant to be FField but found UObject. Property '{0}' '{1}'") },
	{ FNameOverSize, TEXT("String is too long to be converted to FName, which is capped at 1024") },
	{ HeuristicInvalidPointer, TEXT("Pointer memory pattern is likekly invalid: '{0}'") },
	{ ReaderCastExpect, TEXT("Reader cast failed. Expected '{0}, Actual '{1}'") },
	{ WriterCastExpect, TEXT("Writer cast failed. Expected '{0}, Actual '{1}'") },
	{ PeekNoContainerProperty, TEXT("Property Reader/Writer has no container property, Actual: '{0}'"), },

	//	putback reader
	{ CantUsePutbackValue, TEXT("Cannot use putback value in '{0}'") },
	//	pipe visitor
	{ PipeReadWriteMismatch, TEXT("Pipe visit read write mismatch. Reader peeks '{0}' but writer rejects it.") },
	//	skip
	{ SkipOutOfRange, TEXT("Skipping out of range, Container actual length : {0}") },
};

FDcDiagnosticGroup Details = {
	Category,
	DcDimOf(_ReadWriteDetails),
	_ReadWriteDetails
};

} // namespace DcDReadWrite
