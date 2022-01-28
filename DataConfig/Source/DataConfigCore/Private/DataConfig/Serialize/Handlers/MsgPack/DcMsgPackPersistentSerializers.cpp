#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackPersistentSerializers.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"

namespace DcMsgPackHandlers
{

FDcResult HandlerPersistentNameSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Name(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerPersistentTextSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Text(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerPersistentObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentSoftObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentWeakObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeWeakObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentLazyObjectReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeLazyObjectReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentSoftClassReferenceSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftClassReference<&TryWriteObjectReference>(Ctx);
}

FDcResult HandlerPersistentStringSoftObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftObjectToString(Ctx);
}

FDcResult HandlerPersistentStringLazyObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeLazyObjectToString(Ctx);
}

FDcResult HandlerPersistentStringSoftClassSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeSoftClassToString(Ctx);
}

FDcResult HandlerPersistentInstancedSubObjectSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeInstancedSubObject(Ctx);
}

FDcResult HandlerPersistentEnumSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeEnum(Ctx);
}

FDcResult HandlerPersistentFieldPathSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeFieldPathToString(Ctx);
}

} // namespace DcMsgPackHandlers

