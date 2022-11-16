#pragma once

#include "HAL/Platform.h"

namespace DcDReadWrite
{

static const uint16 Category = 0x2;

enum Type : uint16
{
	Unknown = 0,
	InvalidStateNoExpect,
	InvalidStateWithExpect,
	InvalidStateWithExpect2,
	DataTypeMismatch,
	DataTypeMismatch2,
	DataTypeMismatchNoExpect,
	DataTypeMismatchNoCoercion,
	PropertyMismatch,
	PropertyMismatch2,
	AlreadyEnded,
	CantFindPropertyByName,
	WriteClassInlineNotCreated,
	StructNameMismatch,
	ClassNameMismatch,
	EnumNameMismatch,
	EnumNameNotFound,
	EnumValueInvalid,
	EnumSignMismatch,
	EnumMissingEnum,
	WriteBlobOverrun,
	FindPropertyByOffsetFailed,
	DataTypeUnequal,
	DataTypeUnequalLhsRhs,
	ExpectFieldButFoundUObject,
	FNameOverSize,
	HeuristicInvalidPointer,
	ReaderCastExpect,
	WriterCastExpect,
	PeekNoContainerProperty,

	//	property writer
	WriteObjectReferenceDoNotAcceptNull,
	
	//	putback reader
	CantUsePutbackValue,

	//	pipe visitor
	PipeReadWriteMismatch,

	//	skip
	SkipOutOfRange,

};

} // namespace DcDReadWrite

