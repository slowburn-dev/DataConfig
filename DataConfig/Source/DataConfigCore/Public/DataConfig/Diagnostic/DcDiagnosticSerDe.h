#pragma once

#include "HAL/Platform.h"

namespace DcDSerDe
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
	ExpectNumericEntry,
	CoercionFail,
	StructNotFound,
	UObjectTypeMismatch,
	UObjectByStrNotFound,
	ObjectPropertyNotInline,

	//	Deserialize
	RecursiveDeserializeTopPropertyChanged,
	ParseGUIDFailed,

	//	Serialize
	RecursiveSerializeTopPropertyChanged,
	EnumBitFlagsNotFullyMasked,
	SerializerUnexpectedExpandObject,
	SerializerUnexpectedReadReference,

	//	Meta
	MetaKeyMismatch,

	//	Context
	ContextInvalidState,
	ContextReaderNotFound,
	ContextWriterNotFound,
	ContextSerializerNotFound,
	ContextDeserializerNotFound,
	ContextExpectOneProperty,
	ContextExpectOneObject,
	ContextMissingTopObject,

	//	Class
	ClassExpectNonAbstract,
	ClassLhsIsNotChildOfRhs,

	//	Struct
	StructLhsIsNotChildOfRhs,

	//	Pipe
	PipeUnhandledEnded,
	PipeUnhandledExtension,
};

} // namespace DcDSerDe



