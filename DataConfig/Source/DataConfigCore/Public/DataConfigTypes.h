#pragma once

#include "CoreMinimal.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All)

enum class EDataConfigVariantType
{
	Unknown,	// in a bad state
	List,
	Mapping,
	Float,
	Integer,
	String,
	Boolean,
	Nil,
	Blob,
};

DATACONFIGCORE_API void Hello();

//	std::aligned_storage stub
//	https://devdocs.io/cpp/types/aligned_storage
template <size_t _Len, size_t _Align = MIN_ALIGNMENT>
struct AlignedStorage
{
	struct Type
	{
		unsigned char Data[Align(_Len, _Align)];
	};
};



