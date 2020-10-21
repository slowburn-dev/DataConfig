#pragma once

#include "CoreMinimal.h"
#include "Templates/Tuple.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDeserializer : public FNoncopyable
{
	FDcResult Deserialize(FDeserializeContext& Ctx);

	void AddDirectHandler(UClass* PropertyClass, FDeserializeDelegate&& Delegate);
	void AddPredicatedHandler(FDeserializePredicate&& Predicate, FDeserializeDelegate&& Delegate);

	TArray<TTuple<FDeserializePredicate, FDeserializeDelegate>> PredicatedDeserializers;
	TMap<UClass*, FDeserializeDelegate> DirectDeserializersMap;
};


} // namespace DataConfig





