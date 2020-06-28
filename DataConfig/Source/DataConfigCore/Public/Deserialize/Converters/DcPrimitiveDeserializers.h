#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

FResult DATACONFIGCORE_API BoolDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API NameDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API StringDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);


} // namespace DataConfig


