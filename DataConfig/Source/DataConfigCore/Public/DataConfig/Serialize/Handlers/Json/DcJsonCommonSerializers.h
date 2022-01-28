#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcJsonHandlers
{

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsNumericProperty(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNumericSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerBoolSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerNameSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTextSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerEnumSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerStructRootSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerClassRootSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerArraySerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSetSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerMapSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerTransientFieldPathSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerFieldPathSerialize(FDcSerializeContext& Ctx);

	
} // namespace DcJsonHandlers

