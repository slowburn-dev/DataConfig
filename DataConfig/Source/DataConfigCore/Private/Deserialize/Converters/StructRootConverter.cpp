#include "Deserialize/Converters/StructRootConverter.h"
#include "Deserialize/DcDeserializer.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{

bool FStructRootConverter::Prepare(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	RootPeek = Reader.Peek();
	bool bRootPeekPass = RootPeek == EDataEntry::StructRoot
		|| RootPeek == EDataEntry::ClassRoot
		|| RootPeek == EDataEntry::MapRoot;

	bool bWritePass = Writer.Peek(EDataEntry::StructRoot).Ok();
	return bRootPeekPass && bWritePass;
}

FResult FStructRootConverter::Deserialize(FReader& Reader, FPropertyWriter& Writer, FDeserializeContext& Ctx)
{
	if (RootPeek == EDataEntry::MapRoot)
	{
		TRY(Writer.GetWriteProperty(&Ctx.Property));

		TRY(Reader.ReadMapRoot(nullptr));
		TRY(Writer.WriteStructRoot(Ctx.Property->GetFName()));

		EDataEntry CurPeek = Reader.Peek();
		while (CurPeek != EDataEntry::MapEnd)
		{
			//	process key 
			CurPeek = Reader.Peek();
			if (CurPeek == EDataEntry::Name)
			{
				FName Value;
				TRY(Reader.ReadName(&Value, nullptr));
				TRY(Writer.WriteName(Value));
			}
			else if (CurPeek == EDataEntry::String)
			{
				FString Value;
				TRY(Reader.ReadString(&Value, nullptr));
				TRY(Writer.WriteName(FName(*Value)));
			}

			TRY(Writer.GetWriteProperty(&Ctx.Property));
			TRY(Ctx.Deserializer->Deserialize(
				Reader,
				Writer,
				Ctx
			));

			CurPeek = Reader.Peek();
		}

		TRY(Reader.ReadMapEnd(nullptr));
		return Ok();
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}
}


} // namespace DataConfig
