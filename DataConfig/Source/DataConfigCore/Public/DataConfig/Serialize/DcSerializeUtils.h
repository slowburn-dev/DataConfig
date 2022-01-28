#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcSerializeUtils
{

DATACONFIGCORE_API FDcResult RecursiveSerialize(FDcSerializeContext& Ctx);

template<typename TStruct>
EDcSerializePredicateResult PredicateIsUStruct(FDcSerializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == TBaseStructure<TStruct>::Get()
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsSubObjectProperty(FDcSerializeContext& Ctx);

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsEnumProperty(FDcSerializeContext& Ctx);
	
} // namespace DcSerializeUtils

