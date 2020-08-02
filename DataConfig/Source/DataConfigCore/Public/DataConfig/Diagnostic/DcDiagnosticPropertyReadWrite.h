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
		InvalidStateWithExpect2,
		DataTypeMismatch,
		DataTypeMismatch2,
		PropertyMismatch,
		AlreadyEnded,
		CantFindPropertyByName,
		WriteClassInlineNotCreated,
	};

} // namespace DataConfig::DPropertyReadWrite


} // namespace DataConfig
