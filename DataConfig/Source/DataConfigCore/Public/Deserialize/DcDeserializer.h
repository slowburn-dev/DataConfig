#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDeserializer : public FNoncopyable
{
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FPropertyDatum Datum, FDeserializeContext& Ctx);
};


} // namespace DataConfig





