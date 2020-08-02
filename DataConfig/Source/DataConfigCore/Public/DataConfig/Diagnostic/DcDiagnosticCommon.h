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
	NotImplemented = 1,

	Fun = 253,
};

} // namespace DataConfig::DCommon


} // namespace DataConfig
