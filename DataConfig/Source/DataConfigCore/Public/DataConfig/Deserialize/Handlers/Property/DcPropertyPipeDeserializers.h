#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcPropertyPipeHandlers {

DATACONFIGCORE_API FDcResult HandlerScalarDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DATACONFIGCORE_API FDcResult HandlerClassDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcPropertyHandlers

