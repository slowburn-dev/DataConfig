#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPrimitiveConverter : public IDeserializeConverter
{
	bool Prepare(FReader& Reader, FPropertyDatum& Datum, FDeserializeContext& Ctx) override;
	FResult Deserialize(FReader& Reader, FPropertyDatum& Datum, FDeserializeContext& Ctx) override;
};



} // namespace DataConfig

