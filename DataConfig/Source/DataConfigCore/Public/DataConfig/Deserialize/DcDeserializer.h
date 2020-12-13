#pragma once

#include "CoreMinimal.h"
#include "Templates/Tuple.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

struct DATACONFIGCORE_API FDcDeserializer : public FNoncopyable
{
	FDcResult Deserialize(FDcDeserializeContext& Ctx);

	void AddDirectHandler(FFieldClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
	void AddDirectHandler(UClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
	void AddPredicatedHandler(FDcDeserializePredicate&& Predicate, FDcDeserializeDelegate&& Delegate);

	TArray<TTuple<FDcDeserializePredicate, FDcDeserializeDelegate>> PredicatedDeserializers;

	//	TODO [PERF] investigate if ArrayBased map is faster in this case when profiling is setup
	TMap<UClass*, FDcDeserializeDelegate> UClassDeserializerMap;
	TMap<FFieldClass*, FDcDeserializeDelegate> FieldClassDeserializerMap;
};
