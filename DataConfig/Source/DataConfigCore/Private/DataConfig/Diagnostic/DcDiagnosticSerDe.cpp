#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDSerDe
{

struct FDcDiagnosticDetail _DeserializeDetails[] = {
	{ NoMatchingHandler, TEXT("No matching handler, Property: '{0}' '{1}'"), },
	{ MismatchHandler, TEXT("Mismatched handler, Property: '{0}' '{1}'"), },
	{ NotPrepared, TEXT("Context isn't prepared, need to call 'Ctx.Prepare'"), },
	{ DataEntryMismatch, TEXT("Type mismatch, Expect: '{0}', Actual: '{1}'"), },
	{ DataEntryMismatch2, TEXT("Type mismatch, Expect: '{0}'/'{1}', Actual: '{2}'"), },
	{ DataEntryMismatch3, TEXT("Type mismatch, Expect: '{0}'/'{1}'/'{2}', Actual: {3}"), },
	{ ExpectMetaType, TEXT("Expect '$type' but not found")},
	{ ExpectNumericEntry, TEXT("Expect numeric data but not found, Actual: '{0}'")},
	{ CoercionFail, TEXT("Reader coercion failed, Actual: '{0}'")},
	{ StructNotFound, TEXT("Expect struct root or property but not found:, Expect: '{0}', Property: '{1}' '{2}'") },
	{ UObjectTypeMismatch, TEXT("UObject type mismatch, Expect: '{0}, Actual: '{1}'") },
	{ UObjectByStrNotFound, TEXT("Find UObject by string failed, Expected Type: '{0}', Str: '{1}'"), },
	{ ObjectPropertyNotInline, TEXT("Object Property isn't inline property, Property: {0}' '{1}'"), },

	//	Deserialize
	{ RecursiveDeserializeTopPropertyChanged, TEXT("Recursive deserialize detects that top property changed. It's likely a deserialize logic error."), },
	{ ParseGUIDFailed, TEXT("Parse GUID failed, Actual: '{0}'"), },

	//	Serialize
	{ RecursiveSerializeTopPropertyChanged, TEXT("Recursive serialize detects that top property changed. It's likely a deserialize logic error."), },
	{ EnumBitFlagsNotFullyMasked, TEXT("BitFlag enum value can not be fully masked by all enum branches. It's likely the num branchs are changed and the value is stale. Enum '{0}' "), },
	{ SerializerUnexpectedExpandObject, TEXT("Property should be expanded but it's being read as a reference, Property: {0}' '{1}'"), },
	{ SerializerUnexpectedReadReference, TEXT("Property should be read as a reference but it's being expanded, Property: {0}' '{1}'"), },

	//	Meta
	{ MetaKeyMismatch, TEXT("Expect meta key but not found, Expect: '{0}', Actual: '{1}'")},

	//	Context
	{ ContextInvalidState, TEXT("Context invalid internal state, Actual: '{0}'")},
	{ ContextReaderNotFound, TEXT("Context missing 'Reader'")},
	{ ContextWriterNotFound, TEXT("Context missing 'Writer'")},
	{ ContextDeserializerNotFound, TEXT("Context missing 'Serializer'")},
	{ ContextDeserializerNotFound, TEXT("Context missing 'Deserializer'")},
	{ ContextExpectOneProperty, TEXT("Context expect exactly 1 Property on prepare, Actual: '{0}'")},
	{ ContextExpectOneObject, TEXT("Context expect exactly 1 Object on prepare, Actual: '{0}'")},
	{ ContextMissingTopObject, TEXT("Context missing top object")},

	//	Class
	{ ClassExpectNonAbstract, TEXT("Expect class to be non abstract but failed, Class: '{0}'"), },
	{ ClassLhsIsNotChildOfRhs, TEXT("Class '{0}' is not child of '{1}'") },

	//	Struct
	{ StructLhsIsNotChildOfRhs, TEXT("Struct '{0}' is not child of '{1}'") },

	//	Pipe
	{ PipeUnhandledEnded, TEXT("Pipe visit can't handle Ended") },
	{ PipeUnhandledExtension, TEXT("Pipe visit can't handle Extension") },

};

FDcDiagnosticGroup Details = {
	Category,
	DcDimOf(_DeserializeDetails),
	_DeserializeDetails
};

} // namespace DcDSerDe
