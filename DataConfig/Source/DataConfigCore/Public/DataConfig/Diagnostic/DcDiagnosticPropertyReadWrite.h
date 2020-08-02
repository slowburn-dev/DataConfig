#pragma once

#include "HAL/Platform.h"

namespace DataConfig
{

namespace DPropertyReadWrite
{
	static const uint16 Category = 0x2;

	enum Type : uint16
	{
		Unknown = 0,
		InvalidStateNoExpect,
		InvalidStateWithExpect,
		PropertyMismatch,
	};

} // namespace DataConfig::DPropertyReadWrite


} // namespace DataConfig
