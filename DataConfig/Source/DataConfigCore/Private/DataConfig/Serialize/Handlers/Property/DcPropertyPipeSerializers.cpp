#include "DataConfig/Serialize/Handlers/Property/DcPropertyPipeSerializers.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/Serialize/DcSerializeUtils.h"

namespace DcPropertyPipeHandlers {

FDcResult HandlerSetSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		EDcDataEntry::SetEnd,
		&DcSerializeUtils::RecursiveSerialize,
		&FDcPropertyReader::ReadSetRoot,
		&FDcPropertyReader::ReadSetEnd,
		&FDcWriter::WriteSetRoot,
		&FDcWriter::WriteSetEnd
	>(Ctx);
}

FDcResult HandlerMapSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeMap<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

FDcResult HandlerStructSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeStruct<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

FDcResult HandlerClassSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeClass<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

FDcResult HandlerOptionalSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeOptional<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

} // namespace DcPropertyPipeHandlers

