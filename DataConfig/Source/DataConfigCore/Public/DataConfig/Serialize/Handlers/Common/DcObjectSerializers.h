#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

namespace DcCommonHandlers {

///	Serializers that would all use a loaded, in memory `UObject*` even if it's Lazy/Soft/etc.
DATACONFIGCORE_API FDcResult HandlerObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSoftObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerWeakObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerLazyObjectReferenceSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerClassReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSoftClassReferenceSerialize(FDcSerializeContext& Ctx);

///	Serialize Soft/Lazy as string, without loading actual object
DATACONFIGCORE_API FDcResult HandlerSoftObjectToStringSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerLazyObjectToStringSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerSoftClassToStringSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API EDcSerializePredicateResult PredicateIsSubObjectProperty(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerInstancedSubObjectSerialize(FDcSerializeContext& Ctx);


} // namespace DcCommonHandlers



