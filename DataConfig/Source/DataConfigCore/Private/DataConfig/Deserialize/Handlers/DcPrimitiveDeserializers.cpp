#include "DataConfig/Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcHandlers {

FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UBoolProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->ReadNext(&Next));

	if (Next != EDcDataEntry::Bool)
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::Bool << Next;
	}
	DC_TRY(Ctx.Writer->WriteNext(EDcDataEntry::Bool));

	bool Value;
	DC_TRY(Ctx.Reader->ReadBool(&Value));
	DC_TRY(Ctx.Writer->WriteBool(Value));

	OutRet = EDcDeserializeResult::Processed;
	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UNameProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->WriteNext(EDcDataEntry::Name));

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->ReadNext(&Next));
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
			<< EDcDataEntry::Name << EDcDataEntry::String << Next;
	}
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UStrProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->WriteNext(EDcDataEntry::String));

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->ReadNext(&Next));
	if (Next == EDcDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value));
		DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
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

