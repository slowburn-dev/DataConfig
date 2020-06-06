#pragma once

#include "CoreMinimal.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All)

namespace DataConfig
{

DATACONFIGCORE_API uint8* GetZeroBitPattern128();

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

FORCEINLINE FResult Ok() {
	return FResult{0, 0};
}

FORCEINLINE FResult Fail(EErrorCode Status) {
	check((uint32)Status != 0);

#if DO_CHECK
	PLATFORM_BREAK();
#endif

	return FResult{ (uint32)Status, 0 };
}

FORCEINLINE FResult FailIf(bool Cond, EErrorCode Status) {
	check((uint32)Status != 0);
	if (Cond) {
		return Fail(Status);
	}
	else
	{
		return Ok();
	}
}

#define TRY(expr)							\
	do {									\
		::DataConfig::FResult Ret = (expr);	\
		if (!Ret.Ok()) {					\
			return Ret;						\
		}									\
	} while (0)



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

enum class EDataEntry
{
	//	Data Type
	Nil,

	Bool,
	Name,
	String,

	Float,
	Double,

	Int8,
	Int16,
	Int,
	Int64,

	Byte,
	UInt16,
	UInt32,
	UInt64,

	//	Struct
	StructRoot,
	StructEnd,

	//	Class
	ClassRoot,
	ClassEnd,

	//	Map
	MapRoot,
	MapEnd,

	//	Array
	ArrayRoot,
	ArrayEnd,

	//	Reference
	Reference,

	//	Meta
	Ended, // or error or invalid state, 
};

EErrorCode GetReadErrorCode(EDataEntry DataEntry);
EErrorCode GetWriteErrorCode(EDataEntry DataEntry);

struct DATACONFIGCORE_API FContextStorage : private FNoncopyable
{
	using ImplStorageType = TAlignedStorage<64>::Type;
	ImplStorageType ImplStorage;

	template<typename TContext, typename... TArgs>
	TContext& Emplace(TArgs&&... Args)
	{
		check(IsEmpty());
		return (new (this)TContext(Forward<TArgs>(Args)...));
	}

	FContextStorage();
	~FContextStorage();
	bool IsEmpty();
};

} // namespace DataConfig




