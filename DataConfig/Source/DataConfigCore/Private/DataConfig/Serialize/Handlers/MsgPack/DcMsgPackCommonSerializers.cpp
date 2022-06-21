#include "DataConfig/Serialize/Handlers/MsgPack/DcMsgPackCommonSerializers.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/Writer/DcWriter.h"

#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"

namespace DcMsgPackHandlers
{

FDcResult HandlerMapSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeMap<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

EDcSerializePredicateResult PredicateIsBlobProperty(FDcSerializeContext& Ctx)
{
	return Ctx.TopProperty().HasMetaData(DcMsgPackUtils::DC_META_MSGPACK_BLOB)
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

FDcResult HandlerBlobSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Blob(Ctx.Reader, Ctx.Writer);
}

} // namespace DcMsgPackHandlers
