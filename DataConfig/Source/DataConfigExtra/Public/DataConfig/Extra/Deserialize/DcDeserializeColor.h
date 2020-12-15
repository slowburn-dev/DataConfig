#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"

///	Deserialize `FColor` from a hex string like `#FFGGBBAA`

namespace DcExtra {

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra
