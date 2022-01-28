#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DcSerDeBase64.generated.h"

///	blob `TArray<uint8>` <-> Base64 string

namespace DcExtra {

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandleBase64BlobDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API EDcSerializePredicateResult PredicateIsBase64Blob(FDcSerializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandleBase64BlobSerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestStructWithBase64
{
	GENERATED_BODY()

	UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField1;
	UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField2;
};




