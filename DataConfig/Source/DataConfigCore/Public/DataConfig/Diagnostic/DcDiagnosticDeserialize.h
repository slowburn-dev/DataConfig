#pragma once

#include "HAL/Platform.h"

namespace DcDDeserialize
{
	static const uint16 Category = 0x4;

	enum Type : uint16
	{
		Unknown = 0,
		NoMatchingHandler,
		DataEntryMismatch,
		DataEntryMismatch2,
		DataEntryMismatch3,
		ExpectMetaType,
	};

} // namespace DPropertyReadWrite






