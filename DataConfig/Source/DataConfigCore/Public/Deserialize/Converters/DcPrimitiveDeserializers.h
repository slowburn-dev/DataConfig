#pragma once

#include "CoreMinimal.h"
#include "DcTypes.h"
#include "Deserialize/DcDeserializeTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPrimitiveConverter : public IDeserializeConverter
{
	bool Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) override;
	FResult Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx) override;

	EDataEntry CurRead;
};

FResult DATACONFIGCORE_API BoolDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API NameDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);
FResult DATACONFIGCORE_API StringDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet);


} // namespace DataConfig


