#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcDeserializeUtils
{

DATACONFIGCORE_API FDcResult RecursiveDeserialize(FDcDeserializeContext& Ctx);

template<typename TStruct>
EDcDeserializePredicateResult PredicateIsUStruct(FDcDeserializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == TBaseStructure<TStruct>::Get()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}
	
} // namespace DcDeserializeUtils


