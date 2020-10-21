#include "DataConfig/Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DataConfig
{

FDcResult HandlerBoolDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UBoolProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->Peek(EDataEntry::Bool));

	bool Value;
	DC_TRY(Ctx.Reader->ReadBool(&Value, nullptr));
	DC_TRY(Ctx.Writer->WriteBool(Value));

	OutRet = EDeserializeResult::Processed;
	return OkWithProcessed(OutRet);
}

FDcResult HandlerNameDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UNameProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->Peek(EDataEntry::Name));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteName(Value));
		return OkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		return OkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DDeserialize, DataEntryMismatch2))
			<< EDataEntry::Name << EDataEntry::String << Next;
	}
}

FDcResult HandlerStringDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UStrProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	DC_TRY(Ctx.Writer->Peek(EDataEntry::String));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteString(Value.ToString()));
		return OkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value, nullptr));
		DC_TRY(Ctx.Writer->WriteString(Value));
		return OkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DDeserialize, DataEntryMismatch2))
			<< EDataEntry::Name << EDataEntry::String << Next;
	}
}

} // namespace DataConfig

