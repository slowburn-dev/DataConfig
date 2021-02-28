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
	ExpectNumericEntry,
	CoercionFail,
	EnumNameNotFound,
	StructNotFound,
	ClassLhsIsNotChildOfRhs,
	UObjectTypeMismatch,
	UObjectByStrNotFound,
	ObjectPropertyNotInline,

	//	Context
	ContextInvalidState,
	ContextReaderNotFound,
	ContextWriterNotFound,
	ContextDeserializerNotFound,
	ContextExpectOneProperty,
	ContextExpectOneObject,

	//	Class
	ClassExpectNonAbstract,
};

} // namespace DcDDeserialize



