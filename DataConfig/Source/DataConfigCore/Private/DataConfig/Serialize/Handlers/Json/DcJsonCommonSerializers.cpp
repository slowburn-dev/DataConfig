#include "DataConfig/Serialize/Handlers/Json/DcJsonCommonSerializers.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Serialize/DcSerializer.h"

#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerializeCommon.inl"

namespace DcJsonHandlers {

EDcSerializePredicateResult PredicateIsNumericProperty(FDcSerializeContext& Ctx)
{
	return Ctx.TopProperty().IsA<FNumericProperty>()
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;
}

FDcResult HandlerNumericSerialize(FDcSerializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (!DcTypeUtils::IsNumericDataEntry(Next))
		return DC_FAIL(DcDSerDe, ExpectNumericEntry) << Next;

	EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());
	DC_TRY(DcSerDeUtils::DispatchPipeVisit(Actual, Ctx.Reader, Ctx.Writer));
	return DcOk();
}

FDcResult HandlerBoolSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Bool(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerNameSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Name(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerStringSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_String(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerTextSerialize(FDcSerializeContext& Ctx)
{
	return DcPipe_Text(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerEnumSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeEnum(Ctx);
}

FDcResult HandlerStructRootSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerSerializeStructToMap(Ctx);
}

FDcResult HandlerClassRootSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerSerializeClassToMap(Ctx);
}

FDcResult HandlerArraySerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeLinearContainer<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		EDcDataEntry::ArrayEnd,
		&DcSerializeUtils::RecursiveSerialize,
		&FDcPropertyReader::ReadArrayRoot,
		&FDcPropertyReader::ReadArrayEnd,
		&FDcWriter::WriteArrayRoot,
		&FDcWriter::WriteArrayEnd
	>(Ctx);
}

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
		&FDcWriter::WriteArrayRoot,
		&FDcWriter::WriteArrayEnd
	>(Ctx);
}

FDcResult HandlerMapSerialize(FDcSerializeContext& Ctx)
{
	return DcHandlerPipeStringKeyMap<
		FDcSerializeContext,
		FDcPropertyReader,
		FDcWriter,
		&DcSerializeUtils::RecursiveSerialize
	>(Ctx);
}

FDcResult HandlerFieldPathSerialize(FDcSerializeContext& Ctx)
{
	return DcSerializeFieldPathToString(Ctx);
}

} // namespace DcJsonHandlers
