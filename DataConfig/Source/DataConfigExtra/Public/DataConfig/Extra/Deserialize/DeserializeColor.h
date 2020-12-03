#pragma once

/*
 *	Deserialize `FColor` from a hex string like `#FFGGBBAA`
 *
 */

#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcExtra {

EDcDeserializePredicateResult DATACONFIGEXTRA_API PredicateIsColorStruct(FDcDeserializeContext& Ctx);

FDcResult DATACONFIGEXTRA_API HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra
