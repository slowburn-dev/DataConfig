#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackPersistentDeserializers.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"

namespace DcMsgPackHandlers
{

FDcResult HandlerPersistentNameDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Name(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerPersistentTextDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Text(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerPersistentObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentSoftObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentWeakObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeWeakObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentLazyObjectReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeLazyObjectReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentSoftClassReferenceDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeSoftClassReference<TryReadObjectReference>(Ctx);
}

FDcResult HandlerPersistentStringSoftObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToSoftObject(Ctx);
}

FDcResult HandlerPersistentStringLazyObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToLazyObject(Ctx);
}

FDcResult HandlerPersistentStringSoftClassDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToSoftClass(Ctx);
}

FDcResult HandlerPersistentInstancedSubObjectDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeInstancedSubObject<TryReadTypeStr>(Ctx);
}

FDcResult HandlerPersistentEnumDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeEnum(Ctx);
}

FDcResult HandlerPersistentFieldPathDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToFieldPath(Ctx);
}

} // namespace DcMsgPackHandlers


