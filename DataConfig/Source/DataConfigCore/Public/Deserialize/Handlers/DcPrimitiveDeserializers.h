#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

FResult DATACONFIGCORE_API HandlerBoolDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API HandlerNameDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API HandlerStringDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet);


} // namespace DataConfig


