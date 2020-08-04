#pragma once

#include "HAL/Platform.h"

namespace DataConfig
{

namespace DCommon
{

static const uint16 Category = 0x1;

enum Type : uint16
{
	Unknown = 0,
	NotImplemented,
	Unreachable,
	Unexpected1,


	PlaceHoldError,
};

} // namespace DataConfig::DCommon


} // namespace DataConfig
