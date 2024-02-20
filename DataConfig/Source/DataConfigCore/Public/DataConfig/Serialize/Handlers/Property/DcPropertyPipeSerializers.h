#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcPropertyPipeHandlers {

DATACONFIGCORE_API FDcResult HandlerSetSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerMapSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStructSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerClassSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerOptionalSerialize(FDcSerializeContext& Ctx);

} // namespace DcPropertyPipeHandlers

