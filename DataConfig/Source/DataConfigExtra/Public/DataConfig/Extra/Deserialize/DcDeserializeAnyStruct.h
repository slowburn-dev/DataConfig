#pragma once

///	Deserialize arbitrary struct into a `FDcAnyStruct`
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcExtra 
{

EDcDeserializePredicateResult DATACONFIGEXTRA_API PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx);

FDcResult DATACONFIGEXTRA_API HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra



