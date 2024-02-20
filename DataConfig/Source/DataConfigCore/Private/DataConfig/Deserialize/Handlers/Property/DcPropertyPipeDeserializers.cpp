#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/SerDe/DcSerDeCommon.inl"

namespace DcPropertyPipeHandlers {

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		EDcDataEntry::SetEnd,
		&DcDeserializeUtils::RecursiveDeserialize,
		&FDcReader::ReadSetRoot,
		&FDcReader::ReadSetEnd,
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
	return DcHandlerPipeStruct<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

FDcResult HandlerClassDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeClass<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

FDcResult HandlerOptionalDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerPipeOptional<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

} // namespace DcPropertyPipeHandlers


