#pragma once

///	Deserialize arbitrary struct into a `FDcAnyStruct`
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcExtra 
{

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra



