#include "DataConfigTypes.h"

DEFINE_LOG_CATEGORY(LogDataConfigCore);

namespace DataConfig {

uint8* GetUnitializedBitPattern128()
{
	static bool _bInitialized = false;
	static uint8 __BUFFER[128];
	if (!_bInitialized)
	{
		FMemory::Memset(__BUFFER, 0x66, 128);
		_bInitialized = true;
	}
	return __BUFFER;
}

} // namespace DataConfig
