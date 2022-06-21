#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcMsgPackHandlers {

DATACONFIGCORE_API FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsBlobProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerBlobDeserialize(FDcDeserializeContext& Ctx);

} // namespace DcMsgPackHandlers

