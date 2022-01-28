#include "DataConfig/Deserialize/Handlers/Json/DcJsonObjectDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"

namespace DcJsonHandlers {

FDcResult HandlerObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerSoftObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerWeakObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeWeakObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerLazyObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeLazyObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerSoftClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerStringSoftObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToSoftObject(Ctx);
}

FDcResult HandlerStringLazyObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToLazyObject(Ctx);
}

FDcResult HandlerStringSoftClassDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToSoftClass(Ctx);
}

FDcResult HandlerInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeInstancedSubObject<TryReadTypeStr>(Ctx);
}

}	// namespace DcJsonHandlers

