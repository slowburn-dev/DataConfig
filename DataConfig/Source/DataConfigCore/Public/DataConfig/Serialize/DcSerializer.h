#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

struct DATACONFIGCORE_API FDcSerializer : public FNoncopyable
{
	FDcResult Serialize(FDcSerializeContext& Ctx);

	void AddDirectHandler(FFieldClass* PropertyClass, FDcSerializeDelegate&& Delegate);
	void AddDirectHandler(UClass* PropertyClass, FDcSerializeDelegate&& Delegate);
	void AddPredicatedHandler(FDcSerializePredicate&& Predicate, FDcSerializeDelegate&& Delegate);

	TArray<TTuple<FDcSerializePredicate, FDcSerializeDelegate>> PredicatedSerializers;

	TMap<UClass*, FDcSerializeDelegate> UClassSerializerMap;
	TMap<FFieldClass*, FDcSerializeDelegate> FieldClassSerializerMap;
};


