#pragma once

#include "CoreMinimal.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All)

namespace DataConfig
{

enum class EErrorCode : uint32;

struct DATACONFIGCORE_API FResult
{
	uint32 Status : 31;
	uint32 bHasData : 1;

	//	TODO aligned data of 3*4bytes, make it a 16bytes data structure

	FORCEINLINE bool Ok()
	{
		return Status == 0;
	}
};

static FORCEINLINE FResult Ok() {
	return FResult{0, 0};
}

static FORCEINLINE FResult Fail(EErrorCode Status) {
	check((uint32)Status != 0);
	return FResult{ (uint32)Status, 0 };
}



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




