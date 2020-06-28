#include "Deserialize/Converters/DcPrimitiveDeserializers.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{

bool FPrimitiveConverter::Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	/*
	bool bIsPrimitive = Ctx.Property->IsA<UBoolProperty>()
		|| Ctx.Property->IsA<UNameProperty>()
		|| Ctx.Property->IsA<UStrProperty>();

	CurRead = Reader.Peek();
	bool bPeekPass = Writer.Peek(CurRead).Ok();
	return bIsPrimitive && bPeekPass;
	*/
	return true;
}

FResult FPrimitiveConverter::Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	if (CurRead == EDataEntry::Bool)
	{
		bool Value;
		TRY(Reader.ReadBool(&Value, nullptr));
		return Writer.WriteBool(Value);
	}
	else if (CurRead == EDataEntry::Name)
	{
		FName Value;
		TRY(Reader.ReadName(&Value, nullptr));
		return Writer.WriteName(Value);
	}
	else if (CurRead == EDataEntry::String)
	{
		FString Value;
		TRY(Reader.ReadString(&Value, nullptr));
		return Writer.WriteString(Value);
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}

FResult BoolDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
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

FResult NameDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
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

FResult StringDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
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

