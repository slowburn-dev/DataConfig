#include "DataConfig/Serialize/Handlers/Json/DcJsonObjectSerializers.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"

namespace DcJsonHandlers
{

FDcResult HandlerObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerSoftObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerWeakObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeWeakObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerLazyObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeLazyObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerSoftClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerStringSoftObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftObjectToString(Ctx);
}

FDcResult HandlerStringLazyObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeLazyObjectToString(Ctx);
}

FDcResult HandlerStringSoftClassSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftClassToString(Ctx);
}

FDcResult HandlerInstancedSubObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeInstancedSubObject(Ctx);
}

} // namespace DcJsonHandlers




