#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"

///	Deserialize blob `TArray<uint8>` fields from Base64 string

namespace DcExtra {

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra {





