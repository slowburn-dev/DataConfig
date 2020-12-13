#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcHandlers {

DATACONFIGCORE_API FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	// namespace DcHandlers


