#pragma once

#include "HAL/Platform.h"

namespace DataConfig
{

namespace DDeserialize
{
	static const uint16 Category = 0x4;

	enum Type : uint16
	{
		Unknown = 0,
		NoMatchingHandler,
		DataEntryMismatch,
		DataEntryMismatch2,
		DataEntryMismatch3,
	};

	} // namespace DataConfig::DPropertyReadWrite


} // namespace DataConfig





