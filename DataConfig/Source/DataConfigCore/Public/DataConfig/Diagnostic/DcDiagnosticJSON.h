#pragma once

#include "HAL/Platform.h"

namespace DJSON
{
	static const uint16 Category = 0x3;

	enum Type : uint16
	{
		Unknown = 0,
		UnexpectedEnd,
		AlreadyEndedButExpect,
		ExpectWordButNotFound,
		ExpectCharButNotFound,
		UnexpectedChar1,
	};

} // namespace DJSON

