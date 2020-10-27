#pragma once

#include "HAL/Platform.h"

namespace DcDJSON
{
	static const uint16 Category = 0x3;

	enum Type : uint16
	{
		Unknown = 0,
		UnexpectedEnd,
		AlreadyEndedButExpect,
		ExpectWordButNotFound,
		UnexpectedChar,
		UnexpectedToken,
	};

} // namespace DJSON

