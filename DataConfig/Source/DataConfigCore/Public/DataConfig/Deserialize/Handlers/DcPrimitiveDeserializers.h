#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

FDcResult DATACONFIGCORE_API HandlerBoolDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerNameDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FDcResult DATACONFIGCORE_API HandlerStringDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);


} // namespace DataConfig


