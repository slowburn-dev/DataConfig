#pragma once

#include "HAL/Platform.h"

namespace DcDJSON
{

static const uint16 Category = 0x3;

enum Type : uint16
{
	Unknown = 0,
	ExpectWordButNotFound,
	ExpectWordButEOF,
	UnexpectedChar,
	UnexpectedToken,
	UnexpectedEOF,
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
	ExpectStateInProgress,
	ExpectStateUninitializedOrFinished,
	UnexpectedTrailingToken,

	//	Number
	NumberInvalidChar,
	NumberExpectDigitAfterMinus,
	NumberExpectDigitAfterDot,
	NumberExpectSignDigitAfterExp,
	NumberExpectDigitAfterExpSign,

	//	End
	EndUnclosedArray,
	EndUnclosedObject,

	//	Writer
	ExpectStringAtObjectKey,
	UnexpectedArrayEnd,
	UnexpectedObjectEnd,
};

} // namespace DcDJSON

