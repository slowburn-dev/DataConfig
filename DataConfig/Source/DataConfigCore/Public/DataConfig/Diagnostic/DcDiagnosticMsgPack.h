#pragma once

#include "HAL/Platform.h"

namespace DcDMsgPack
{

static const uint16 Category = 0x5;
	
enum Type : uint16
{
	Unknown = 0,

	TypeByteMismatch,
	TypeByteMismatchNoExpect,
	UnexpectedArrayEnd,
	UnexpectedMapEnd,
	ReadPassArrayEnd,
	ReadPassMapEnd,

	//	Reader
	UnknownMsgTypeByte,
	ReadingPastEnd,
	SizeOverInt32Max,
	ArrayRemains,
	MapRemains,
	
};

} // namespace DcDMsgPack

