#pragma once

#include "HAL/Platform.h"

namespace DcDCommon
{

static const uint16 Category = 0x1;

enum Type : uint16
{
	Unknown = 0,
	Unhandled = 1,	// hard coded 1
	NotImplemented,
	Unreachable,
	Unexpected1,


	PlaceHoldError,
};

} // namespace DCommon

