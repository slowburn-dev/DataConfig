#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcCommonHandlers {

DATACONFIGCORE_API void AddNumericPipeDirectHandlers(FDcDeserializer& Deserializer);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsScalarArrayProperty(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeScalarDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeBoolDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeNameDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeStringDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeTextDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeFloatDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeDoubleDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeInt8Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt16Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt32Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeInt64Deserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPipeUInt8Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt16Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt32Deserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPipeUInt64Deserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerStringToEnumDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringToFieldPathDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerMapToStructDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerMapToClassDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerArrayToSetDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerMapOrArrayOfKeyValueDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerOptionalDeserialize(FDcDeserializeContext& Ctx);

} // namespace DcCommonHandlers
