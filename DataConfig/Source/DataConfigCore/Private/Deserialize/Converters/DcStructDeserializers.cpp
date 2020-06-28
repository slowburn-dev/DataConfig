#include "Deserialize/Converters/DcStructDeserializers.h"
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
		//TRY(Writer.GetWriteProperty(&Ctx.Property));

		TRY(Reader.ReadMapRoot(nullptr));
		//TRY(Writer.WriteStructRoot(Ctx.Property->GetFName()));

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

			//TRY(Writer.GetWriteProperty(&Ctx.Property));
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

FResult DATACONFIGCORE_API StructRootDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDataEntry::StructRoot
		|| Next == EDataEntry::ClassRoot
		|| Next == EDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDataEntry::StructRoot).Ok();
	if (!bRootPeekPass
		|| !bWritePass)
	{
		return OkWithCanNotProcess(OutRet);
	}

	if (Next == EDataEntry::MapRoot)
	{
		//	TODO this is redundant, TopProperty must already be this
		FScopedProperty ScopedStructRootProperty(Ctx);
		TRY(ScopedStructRootProperty.PushProperty());

		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		TRY(Ctx.Writer->WriteStructRoot(ScopedStructRootProperty.Property->GetFName()));

		EDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDataEntry::MapEnd)
		{
			if (CurPeek == EDataEntry::Name)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FName Value;
				TRY(Ctx.Reader->ReadName(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(Value));
			}
			else if (CurPeek == EDataEntry::String)
			{
				TRY(Ctx.Writer->Peek(EDataEntry::Name));

				FString Value;
				TRY(Ctx.Reader->ReadString(&Value, nullptr));
				TRY(Ctx.Writer->WriteName(FName(*Value)));
			}
			else
			{
				return Fail(EErrorCode::DeserializeTypeNotMatch);
			}

			FScopedProperty ScopedValueProperty(Ctx);
			TRY(ScopedValueProperty.PushProperty());
			TRY(Ctx.Deserializer->Deserialize(Ctx));

			CurPeek = Ctx.Reader->Peek();
		}

		TRY(Ctx.Reader->ReadMapEnd(nullptr));
		TRY(Ctx.Writer->WriteStructEnd(ScopedStructRootProperty.Property->GetFName()));

		return OkWithProcessed(OutRet);
	}
	else 
	{
		return Fail(EErrorCode::UnknownError);
	}
}

} // namespace DataConfig
