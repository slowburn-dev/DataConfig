#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "DataConfigTypes.h"

namespace DataConfig
{

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


