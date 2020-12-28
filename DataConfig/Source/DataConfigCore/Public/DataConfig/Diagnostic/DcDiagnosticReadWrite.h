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
	PropertyMismatch,
	AlreadyEnded,
	CantFindPropertyByName,
	WriteClassInlineNotCreated,
	StructNameMismatch,
	ClassNameMismatch,
	EnumNameMismatch,
	EnumNameNotFound,
	EnumValueInvalid,
	EnumSignMismatch,
	WriteBlobOverrun,
	FindPropertyByOffsetFailed,
	
	//	putback reader
	CantUsePutbackValue,

	//	pipe visitor
	PipeReadWriteMismatch,
};

} // namespace DPropertyReadWrite

