#include "DataConfig/Deserialize/Handlers/Json/DcJsonCommonDeserializers.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "UObject/TextProperty.h"

#include "DataConfig/SerDe/DcSerDeUtils.inl"
#include "DataConfig/SerDe/DcSerDeCommon.inl"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"

namespace DcJsonHandlers {

EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx)
{
	return Ctx.TopProperty().IsA<FNumericProperty>()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (!DcTypeUtils::IsNumericDataEntry(Next))
		return DC_FAIL(DcDSerDe, ExpectNumericEntry) << Next;

	//	coercion by property type
	EDcDataEntry Actual = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());
	bool bCanCoerce;
	DC_TRY(Ctx.Reader->Coercion(Actual, &bCanCoerce));
	if (Next != Actual && !bCanCoerce)
		return DC_FAIL(DcDSerDe, CoercionFail) << Next;;

	DC_TRY(DcSerDeUtils::DispatchPipeVisit(Actual, Ctx.Reader, Ctx.Writer));
	return DcOk();
}

FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Bool(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Name(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_String(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx)
{
	return DcPipe_Text(Ctx.Reader, Ctx.Writer);
}

FDcResult HandlerEnumDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeEnum(Ctx);
}

FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeMapToStruct(Ctx);
}

FDcResult HandlerClassRootDeserialize(FDcDeserializeContext& Ctx)
{
	return DcHandlerDeserializeMapToClass(Ctx);
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
	return DcHandlerPipeStringKeyMap<
		FDcDeserializeContext,
		FDcReader,
		FDcPropertyWriter,
		&DcDeserializeUtils::RecursiveDeserialize
	>(Ctx);
}

FDcResult HandlerFieldPathDeserialize(FDcDeserializeContext& Ctx)
{
	return DcDeserializeStringToFieldPath(Ctx);
}

}	// namespace DcJsonHandlers

