#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FStructRootConverter : public IDeserializeConverter
{
	bool Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) override;
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) override;

	EDataEntry RootPeek;
};


} // namespace DataConfig





