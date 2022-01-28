#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"


namespace DcMsgPackHandlers
{

DATACONFIGCORE_API FDcResult HandlerPersistentNameSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentTextSerialize(FDcSerializeContext& Ctx);

///	Serializers that would all use a loaded, in memory `UObject*` even if it's Lazy/Soft/etc.
DATACONFIGCORE_API FDcResult HandlerPersistentObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentSoftObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentWeakObjectReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentLazyObjectReferenceSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPersistentClassReferenceSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentSoftClassReferenceSerialize(FDcSerializeContext& Ctx);

///	Serialize Soft/Lazy as string, without loading actual object
DATACONFIGCORE_API FDcResult HandlerPersistentStringSoftObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentStringLazyObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentStringSoftClassSerialize(FDcSerializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPersistentInstancedSubObjectSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentEnumSerialize(FDcSerializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentFieldPathSerialize(FDcSerializeContext& Ctx);
	
} // namespace DcMsgPackHandlers

