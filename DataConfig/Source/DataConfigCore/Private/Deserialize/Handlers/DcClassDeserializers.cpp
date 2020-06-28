#include "Deserialize/Handlers/DcClassDeserializers.h"
#include "Deserialize/DcDeserializer.h"
#include "Reader/DcReader.h"
#include "Property/DcPropertyWriter.h"

namespace DataConfig
{

FResult DATACONFIGCORE_API DataConfig::ClassRootDeserializeHandler(FDeserializeContext& Ctx, EDeserializeResult& OutRet)
{
	EDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDataEntry::StructRoot
		|| Next == EDataEntry::ClassRoot
		|| Next == EDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDataEntry::ClassRoot).Ok();
	bool bPropertyPass = Ctx.TopProperty()->IsA<UClass>();

	if (!(bRootPeekPass && bWritePass && bPropertyPass))
	{
		return OkWithCanNotProcess(OutRet);
	}

	if (Next == EDataEntry::MapRoot)
	{
		TRY(Ctx.Reader->ReadMapRoot(nullptr));
		FClassPropertyStat WriteClassStat{ Ctx.TopProperty()->GetFName(), EDataReference::InlineObject };
		TRY(Ctx.Writer->WriteClassRoot(WriteClassStat));

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
		TRY(Ctx.Writer->WriteClassEnd(WriteClassStat));

		return OkWithProcessed(OutRet);
	}
	else
	{
		return Fail(EErrorCode::UnknownError);
	}

	return Ok();
}


} // namespace DataConfig
