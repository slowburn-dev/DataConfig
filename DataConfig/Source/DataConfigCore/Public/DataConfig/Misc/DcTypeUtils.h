#pragma once

#include "DataConfig/DcTypes.h"

namespace FDcTypeUtils {

	FORCEINLINE bool IsNumericDataEntry(const EDcDataEntry& Entry)
	{
		return Entry == EDcDataEntry::Int8
			|| Entry == EDcDataEntry::Int16
			|| Entry == EDcDataEntry::Int32
			|| Entry == EDcDataEntry::Int64
			|| Entry == EDcDataEntry::UInt8
			|| Entry == EDcDataEntry::UInt16
			|| Entry == EDcDataEntry::UInt32
			|| Entry == EDcDataEntry::UInt64
			|| Entry == EDcDataEntry::Float
			|| Entry == EDcDataEntry::Double;
	}

}	// namespace FDcTypeUtils

