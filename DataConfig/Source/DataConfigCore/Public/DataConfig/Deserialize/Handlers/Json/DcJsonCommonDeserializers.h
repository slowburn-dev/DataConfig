#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcJsonHandlers {

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerEnumDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerFieldPathDeserialize(FDcDeserializeContext& Ctx);



} // namespace DcJsonHandlers