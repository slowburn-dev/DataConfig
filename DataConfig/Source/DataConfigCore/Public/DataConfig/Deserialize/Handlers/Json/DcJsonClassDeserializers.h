#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

DATACONFIGCORE_API FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGCORE_API FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGCORE_API FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	// namespace DcHandlers
