#include "Reader/Visitor.h"

namespace DataConfig {

FContextStorage::FContextStorage()
{
	FMemory::Memzero(ImplStorage.Data, sizeof(FContextStorage));
}

FContextStorage::~FContextStorage()
{
	//	TODO might need virtual destructor
}

bool FContextStorage::IsEmpty()
{
	return FMemory::Memcmp(ImplStorage.Data, GetZeroBitPattern128(), sizeof(FContextStorage)) == 0;
}

} // namespace DataConfig

