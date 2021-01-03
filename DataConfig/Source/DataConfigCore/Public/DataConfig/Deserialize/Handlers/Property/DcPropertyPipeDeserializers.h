#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcPropertyHandlers {

DATACONFIGCORE_API FDcResult HandlerScalarDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcPropertyHandlers

