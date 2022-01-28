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

FDcResult HandlerScalarDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeScalar(Ctx);
}

FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		EDcDataEntry::ArrayEnd,
		&DcDeserializeUtils::RecursiveDeserialize,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteArrayRoot,
		&FDcPropertyWriter::WriteArrayEnd
	>(Ctx);
}

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		EDcDataEntry::ArrayEnd,
		&DcDeserializeUtils::RecursiveDeserialize,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteSetRoot,
		&FDcPropertyWriter::WriteSetEnd
	>(Ctx);
}

FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeMap<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

FDcResult HandlerStructDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeMapToStruct(Ctx);
}

FDcResult HandlerClassDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeMapToClass(Ctx);
}

EDcDeserializePredicateResult PredicateIsBlobProperty(FDcDeserializeContext& Ctx)
{
	return Ctx.TopProperty().HasMetaData(DcMsgPackUtils::DC_META_MSGPACK_BLOB)
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerBlobDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Blob(Ctx.Reader, Ctx.Writer);
}

} // namespace DcMsgPackHandlers
