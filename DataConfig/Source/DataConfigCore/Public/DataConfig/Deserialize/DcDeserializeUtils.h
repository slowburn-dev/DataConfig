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

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx);
	
} // namespace DcDeserializeUtils


