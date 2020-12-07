#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcHandlers {

FDcResult DATACONFIGCORE_API HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerSetDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerMapDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);


}	// namespace DcHandlers



