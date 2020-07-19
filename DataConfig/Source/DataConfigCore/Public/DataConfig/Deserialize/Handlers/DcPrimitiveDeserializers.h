#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

FResult DATACONFIGCORE_API HandlerBoolDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API HandlerNameDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API HandlerStringDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);


} // namespace DataConfig


