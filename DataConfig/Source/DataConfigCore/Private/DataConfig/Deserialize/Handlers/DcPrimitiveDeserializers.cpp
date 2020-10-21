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

	DC_TRY(Ctx.Writer->Peek(EDataEntry::Bool));

	bool Value;
	DC_TRY(Ctx.Reader->ReadBool(&Value, nullptr));
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

	DC_TRY(Ctx.Writer->Peek(EDataEntry::Name));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteName(Value));
		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch2))
			<< EDataEntry::Name << EDataEntry::String << Next;
	}
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UStrProperty>())
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->Peek(EDataEntry::String));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
		return DcOkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteString(Value));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch2))
			<< EDataEntry::Name << EDataEntry::String << Next;
	}
}

}	// namespace DcHandlers

