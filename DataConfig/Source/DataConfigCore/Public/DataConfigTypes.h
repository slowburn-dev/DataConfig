#pragma once

#include "CoreMinimal.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All)

namespace DataConfig
{

//	std::aligned_storage stub
//	https://devdocs.io/cpp/types/aligned_storage
template <size_t _Len, size_t _Align = MIN_ALIGNMENT>
struct TAlignedStorage
{
	struct Type
	{
		unsigned char Data[Align(_Len, _Align)];
	};
};

} // namespace DataConfig




