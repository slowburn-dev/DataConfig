#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcHandlers {

//	Reads `/Path/To/BP` or `ClassName`
//FResult DATACONFIGCORE_API ClassReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

FDcResult DATACONFIGCORE_API HandlerClassRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

FDcResult DATACONFIGCORE_API HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

EDcDeserializePredicateResult DATACONFIGCORE_API PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);
FDcResult DATACONFIGCORE_API HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	// namespace DcHandlers
