#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DcDDeserialize
{

struct FDcDiagnosticDetail _DeserializeDetails[] = {
	{ NoMatchingHandler, TEXT("No matching handler, Property: '{0}' '{1}'"), },
	{ MismatchHandler, TEXT("Mismatched handler, Property: '{0}' '{1}'"), },
	{ NotPrepared, TEXT("Deserialize isn't prepared, need to call 'Ctx.Prepare'"), },
	{ DataEntryMismatch, TEXT("Deserialize type mismatch, Expect: '{0}', Actual: '{1}'"), },
	{ DataEntryMismatch2, TEXT("Deserialize type mismatch, Expect: '{0}'/'{1}', Actual: '{2}'"), },
	{ DataEntryMismatch3, TEXT("Deserialize type mismatch, Expect: '{0}'/'{1}'/'{2}', Actual: {3}"), },
	{ ExpectMetaType, TEXT("Expect '$type' but not found")},
	{ PropertyMismatch, TEXT("Property type mismatch, Expect: '{0}', Actual: '{1}'"), },
	{ UObjectByNameNotFound, TEXT("Find object by name failed, Actual: '{1}'"), },
	{ ExpectNumericEntry, TEXT("Expect numeric data but not found, Actual: '{1}'")},
	{ CoercionFail, TEXT("Reader coercion failed, Actual: '{0}'")},
	{ EnumNameNotFound, TEXT("Enum name not found in enum type: '{0}', Actual: '{1}'")},
	{ HandlerNotWritingDeserializeResult, TEXT("Deserialize handler forgot to write to 'EDcDeserializeResult& OutRet'") },
	{ StructNotFound, TEXT("Expect struct root or property but not found:, Expect: {0}, Actual: {1}") },

	//	Context
	{ ContextInvalidState, TEXT("Deserializer context invalid internal state, Actual: '{0}'")},
	{ ContextReaderNotFound, TEXT("Deserializer context missing 'Reader'")},
	{ ContextWriterNotFound, TEXT("Deserializer context missing 'Writer'")},
	{ ContextDeserializerNotFound, TEXT("Deserializer context missing 'Deserializer'")},
	{ ContextExpectOneProperty, TEXT("Deserializer context expect exactly 1 Property on prepare, Actual: '{0}'")},
	{ ContextExpectOneObject, TEXT("Deserializer context expect exactly 1 Object on prepare, Actual: '{0}'")},

};

FDcDiagnosticGroup Details = {
	DcDDeserialize::Category,
	DcDimOf(_DeserializeDetails),
	_DeserializeDetails
};

} // namespace DcDDeserialize
