#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcCommonHandlers {

DATACONFIGCORE_API void AddNumericPipeDirectHandlers(FDcSerializer& Serializer);

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsScalarArrayProperty(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeScalarSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeBoolSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeNameSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeStringSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeTextSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeFloatSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeDoubleSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeInt8Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt16Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt32Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt64Serialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeUInt8Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt16Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt32Serialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt64Serialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsEnumProperty(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerEnumToStringSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerFieldPathToStringDeserialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerStructToMapSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerClassToMapSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerArraySerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSetToArraySerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringKeyMapOrArrayOfKeyValueSerialize(FDcSerializeContext& Ctx);
	
} // namespace DcCommonHandlers
