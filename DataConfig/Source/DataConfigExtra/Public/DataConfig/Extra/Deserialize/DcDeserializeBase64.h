#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeBase64.generated.h"

///	Deserialize blob `TArray<uint8>` fields from Base64 string

namespace DcExtra {

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestStructWithBase64
{
	GENERATED_BODY()

	UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField1;
	UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField2;
};




