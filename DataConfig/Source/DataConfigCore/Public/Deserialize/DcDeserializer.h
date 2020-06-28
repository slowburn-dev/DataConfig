#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDeserializer : public FNoncopyable
{
	FResult Deserialize(FDeserializeContext& Ctx);

	void AddDirectHandler(UClass* PropertyClass, FDeserializeDelegate&& Delegate);
	TMap<UClass*, FDeserializeDelegate> DirectDeserializersMap;
};


} // namespace DataConfig





