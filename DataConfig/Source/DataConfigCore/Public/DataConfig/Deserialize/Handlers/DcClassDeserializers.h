#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

//	Reads `/Path/To/BP` or `ClassName`
//FResult DATACONFIGCORE_API ClassReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FResult DATACONFIGCORE_API HandlerClassRootDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FResult DATACONFIGCORE_API HandlerObjectReferenceDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

EDeserializePredicateResult DATACONFIGCORE_API PredicateIsSubObjectProperty(FDeserializeContext& Ctx);
FResult DATACONFIGCORE_API HandlerInstancedSubObjectDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);





} // namespace DataConfig






