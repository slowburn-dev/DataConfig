#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPrimitiveConverter : public IDeserializeConverter
{
	bool Prepare(FReader& Reader, FPropertyWriter& Writer, FPropertyDatum Datum, FDeserializeContext& Ctx) override;
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FPropertyDatum Datum, FDeserializeContext& Ctx) override;

	EDataEntry CurRead;
};

} // namespace DataConfig


