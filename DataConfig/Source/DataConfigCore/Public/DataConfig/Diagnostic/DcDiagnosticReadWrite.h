#pragma once

#include "HAL/Platform.h"


namespace DcDReadWrite
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
		StructNameMismatch,
		UInt64EnumNotSupported,
		
		//	putback reader
		CantUsePutbackValue,
	};

} // namespace DPropertyReadWrite

