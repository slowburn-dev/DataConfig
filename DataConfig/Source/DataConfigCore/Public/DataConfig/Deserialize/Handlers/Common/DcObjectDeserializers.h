#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcCommonHandlers {

///	Deserializers that would all use a loaded, in memory `UObject*` even if it's lazy/soft/etc.
DATACONFIGCORE_API FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSoftObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerWeakObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerLazyObjectReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSoftClassReferenceDeserialize(FDcDeserializeContext& Ctx);

///	Deserialize Soft/Lazy as string, without loading actual object
DATACONFIGCORE_API FDcResult HandlerStringToSoftObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringToLazyObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerStringToSoftClassDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API EDcDeserializePredicateResult PredicateIsSubObjectProperty(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx);

} // namespace DcCommonHandlers

