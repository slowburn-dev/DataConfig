#pragma once

#include "CoreMinimal.h"

DATACONFIGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDataConfigCore, Log, All)

namespace DataConfig
{
	
//	global initializer and shutdown
enum class EInitializeAction
{
	Minimal,
	SetAsConsole,
};

DATACONFIGCORE_API void StartUp(EInitializeAction InAction = EInitializeAction::Minimal);
DATACONFIGCORE_API void ShutDown();
DATACONFIGCORE_API bool IsInitialized();

struct DATACONFIGCORE_API FErrorCode
{
	uint16 CategoryID;
	uint16 ErrorID;
};

struct DATACONFIGCORE_API FResult
{
	//	TODO when benchmark is setup, try figure out whether
	//		 changing this to uint32 affects anything
	enum class EStatus : uint8
	{
		Ok,
		Error
	};

	EStatus Status;

	FORCEINLINE bool Ok()
	{
		return Status == EStatus::Ok;
	}
};

FORCEINLINE FResult Ok() {
	return FResult{ FResult::EStatus::Ok };
}

FORCEINLINE FResult Fail() {
	return FResult{ FResult::EStatus::Error };
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
	Nil,	 //	!! note that this actually doesn't exist in UE4 property data model

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

enum class EDataReference
{
	NullReference,
	ExternalReference,
	ExpandObject,
};

struct FClassPropertyStat
{
	FName Name;
	EDataReference Reference;
};

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




