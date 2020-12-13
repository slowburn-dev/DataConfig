#include "DataConfig/Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"

namespace DcHandlers {

EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx)
{
	return Ctx.TopProperty().IsA<FNumericProperty>()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FNumericProperty>())
		return DcOkWithCanNotProcess(OutRet);

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (!DcTypeUtils::IsNumericDataEntry(Next))
		return DC_FAIL(DcDDeserialize, ExpectNumericEntry) << Next;

	//	property writer driven coercion
	Next = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());
	if (!Ctx.Reader->Coercion(Next))
		return DC_FAIL(DcDDeserialize, CoercionFail) << Next;;

	DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer);

	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FBoolProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next != EDcDataEntry::Bool)
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::Bool << Next;
	}

	bool Value;
	DC_TRY(Ctx.Reader->ReadBool(&Value));
	DC_TRY(Ctx.Writer->WriteBool(Value));

	OutRet = EDcDeserializeResult::Processed;
	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FNameProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value));
		DC_TRY(Ctx.Writer->WriteName(Value));
		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
			<< EDcDataEntry::Name << EDcDataEntry::String << Next
			<< *Ctx.Reader;	// TODO [MULHIGHLIGHT]
	}
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FStrProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value));
		DC_TRY(Ctx.Writer->WriteString(DcPropertyUtils::SafeNameToString(Value)));
		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteString(Value));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
			<< EDcDataEntry::Name << EDcDataEntry::String << Next;
	}
}

}	// namespace DcHandlers

