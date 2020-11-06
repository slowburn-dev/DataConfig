#pragma once

#include "HAL/Platform.h"

namespace DcDJSON
{
	static const uint16 Category = 0x3;

	enum Type : uint16
	{
		Unknown = 0,
		ExpectWordButNotFound,
		UnexpectedChar,
		UnexpectedToken,
		UnclosedBlockComment,
		UnclosedStringLiteral,
		InvalidStringEscaping,
		InvalidControlCharInString,
		ReadUnsignedWithNegativeNumber,
		ParseIntegerFailed,
		DuplicatedKey,
		KeyMustBeString,
		ReadTypeMismatch,
		ExpectComma,
	};

} // namespace DJSON

