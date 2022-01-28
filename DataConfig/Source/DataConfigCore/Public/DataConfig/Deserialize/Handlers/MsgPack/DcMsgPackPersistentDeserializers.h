#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

namespace DcMsgPackHandlers
{

DATACONFIGCORE_API FDcResult HandlerPersistentNameDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentTextDeserialize(FDcDeserializeContext& Ctx);

///	Serializers that would all use a loaded, in memory `UObject*` even if it's Lazy/Soft/etc.
DATACONFIGCORE_API FDcResult HandlerPersistentObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentSoftObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentWeakObjectReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentLazyObjectReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPersistentClassReferenceDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentSoftClassReferenceDeserialize(FDcDeserializeContext& Ctx);

///	Serialize Soft/Lazy as string, without loading actual object
DATACONFIGCORE_API FDcResult HandlerPersistentStringSoftObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentStringLazyObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentStringSoftClassDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGCORE_API FDcResult HandlerPersistentInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentEnumDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGCORE_API FDcResult HandlerPersistentFieldPathDeserialize(FDcDeserializeContext& Ctx);
	
} // namespace DcMsgPackHandlers

