#include "DataConfigTypes.h"

DEFINE_LOG_CATEGORY(LogDataConfigCore);

namespace DataConfig {

uint8* GetZeroBitPattern128()
{
	static uint8 __BUFFER[128] = {0};
	return __BUFFER;
}

} // namespace DataConfig
