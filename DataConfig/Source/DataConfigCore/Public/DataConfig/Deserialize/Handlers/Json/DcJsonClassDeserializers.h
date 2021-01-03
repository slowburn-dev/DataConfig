#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

//	Reads `/Path/To/BP` or `ClassName`
//FResult DATACONFIGCORE_API ClassReferenceDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);

DATACONFIGCORE_API FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGCORE_API FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	// namespace DcHandlers
