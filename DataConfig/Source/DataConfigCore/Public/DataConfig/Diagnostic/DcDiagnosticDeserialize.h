#pragma once

#include "HAL/Platform.h"

namespace DcDDeserialize
{

static const uint16 Category = 0x4;

enum Type : uint16
{
	Unknown = 0,
	NotPrepared,
	NoMatchingHandler,
	MismatchHandler,
	DataEntryMismatch,
	DataEntryMismatch2,
	DataEntryMismatch3,
	ExpectMetaType,
	PropertyMismatch,
	UObjectByNameNotFound,
	ExpectNumericEntry,
	CoercionFail,
	EnumNameNotFound,
	HandlerNotWritingDeserializeResult,
	StructNotFound,
	ClassLhsIsNotChildOfRhs,

	//	Context
	ContextInvalidState,
	ContextReaderNotFound,
	ContextWriterNotFound,
	ContextDeserializerNotFound,
	ContextExpectOneProperty,
	ContextExpectOneObject,
};

} // namespace DcDDeserialize



