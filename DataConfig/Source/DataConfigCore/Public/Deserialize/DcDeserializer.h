#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDeserializer : public FNoncopyable
{
	//	this is the recursvei entrant
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx);
};


} // namespace DataConfig





