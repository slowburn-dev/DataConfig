#include "Deserialize/Converters/PrimitiveConverter.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{

bool FPrimitiveConverter::Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	bool bIsPrimitive = Ctx.Property->IsA<UBoolProperty>()
		|| Ctx.Property->IsA<UNameProperty>()
		|| Ctx.Property->IsA<UStrProperty>();

	CurRead = Reader.Peek();
	bool bPeekPass = Writer.Peek(CurRead).Ok();
	return bIsPrimitive && bPeekPass;
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

} // namespace DataConfig

