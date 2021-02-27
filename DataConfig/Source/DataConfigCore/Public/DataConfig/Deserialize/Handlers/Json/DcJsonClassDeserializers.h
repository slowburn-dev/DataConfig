#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

DATACONFIGCORE_API FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx);

}	// namespace DcHandlers
