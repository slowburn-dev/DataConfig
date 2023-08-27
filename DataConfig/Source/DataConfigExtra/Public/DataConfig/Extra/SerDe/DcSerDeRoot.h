#pragma once

/// NDJSON and root object examples
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Property/DcPropertyUtils.h"

namespace DcExtra
{

DATACONFIGEXTRA_API FDcResult HandlerRootObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGEXTRA_API FDcResult HandlerRootObjectSerialize(FDcSerializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerRootArrayDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGEXTRA_API FDcResult HandlerRootArraySerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

