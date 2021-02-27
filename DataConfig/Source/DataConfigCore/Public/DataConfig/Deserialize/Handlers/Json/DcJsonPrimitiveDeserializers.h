#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerEnumDeserialize(FDcDeserializeContext& Ctx);

}	// namespace DcHandlers
