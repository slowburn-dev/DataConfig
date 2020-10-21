#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

FDcResult DATACONFIGCORE_API HandlerBoolDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerNameDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerStringDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);


