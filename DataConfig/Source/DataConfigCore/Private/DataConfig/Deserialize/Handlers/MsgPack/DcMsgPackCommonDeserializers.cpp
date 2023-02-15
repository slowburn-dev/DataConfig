#include "DataConfig/Deserialize/Handlers/MsgPack/DcMsgPackCommonDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"

#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"

namespace DcMsgPackHandlers {

FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeMap<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

EDcDeserializePredicateResult PredicateIsBlobProperty(FDcDeserializeContext& Ctx)
{
#if WITH_EDITORONLY_DATA
	return Ctx.TopProperty().HasMetaData(DcMsgPackUtils::DC_META_MSGPACK_BLOB)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
#else
	return EDcDeserializePredicateResult::Pass;
#endif // WITH_EDITORONLY_DATA

}

FDcResult HandlerBlobDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Blob(Ctx.Reader, Ctx.Writer);
}

} // namespace DcMsgPackHandlers
