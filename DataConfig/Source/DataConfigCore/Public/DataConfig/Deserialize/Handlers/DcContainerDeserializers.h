#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcHandlers {

DATACONFIGCORE_API FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);


}	// namespace DcHandlers



