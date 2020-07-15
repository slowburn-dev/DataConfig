#include "Deserialize/Handlers/DcPrimitiveDeserializers.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{

FResult HandlerBoolDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UBoolProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	TRY(Ctx.Writer->Peek(EDataEntry::Bool));

	bool Value;
	TRY(Ctx.Reader->ReadBool(&Value, nullptr));
	TRY(Ctx.Writer->WriteBool(Value));

	OutRet = EDeserializeResult::Processed;
	return OkWithProcessed(OutRet);
}

FResult HandlerNameDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UNameProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	TRY(Ctx.Writer->Peek(EDataEntry::Name));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		TRY(Ctx.Reader->ReadName(&Value, nullptr));
		TRY(Ctx.Writer->WriteName(Value));
		return OkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		TRY(Ctx.Reader->ReadString(&Value, nullptr));
		TRY(Ctx.Writer->WriteName(FName(*Value)));
		return OkWithProcessed(OutRet);
	}
	else
	{
		return Fail(EErrorCode::DeserializeTypeNotMatch);
	}
}

FResult HandlerStringDeserialize(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty()->IsA<UStrProperty>())
	{
		return OkWithCanNotProcess(OutRet);
	}

	TRY(Ctx.Writer->Peek(EDataEntry::String));

	EDataEntry Next = Ctx.Reader->Peek();
	if (Next == EDataEntry::Name)
	{
		FName Value;
		TRY(Ctx.Reader->ReadName(&Value, nullptr));
		TRY(Ctx.Writer->WriteString(Value.ToString()));
		return OkWithProcessed(OutRet);
	}
	else if (Next == EDataEntry::String)
	{
		FString Value;
		TRY(Ctx.Reader->ReadString(&Value, nullptr));
		TRY(Ctx.Writer->WriteString(Value));
		return OkWithProcessed(OutRet);
	}
	else
	{
		return Fail(EErrorCode::DeserializeTypeNotMatch);
	}
}

} // namespace DataConfig

