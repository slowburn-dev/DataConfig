#include "DataConfig/Deserialize/Handlers/DcStructDeserializers.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcHandlers {

static FName GetStructName(UField* Property)
{
	if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		return StructProperty->Struct->GetFName();
	}
	else if (UScriptStruct* StructClass = Cast<UScriptStruct>(Property))
	{
		return StructClass->GetFName();
	}
	else
	{
		return FName();
	}
}

FDcResult DATACONFIGCORE_API HandlerStructRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next = Ctx.Reader->Peek();
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->Peek(EDcDataEntry::StructRoot).Ok();
	if (!bRootPeekPass
		|| !bWritePass)
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot(nullptr));
		DC_TRY(Ctx.Writer->WriteStructRoot(GetStructName(Ctx.TopProperty())));

		EDcDataEntry CurPeek = Ctx.Reader->Peek();
		while (CurPeek != EDcDataEntry::MapEnd)
		{
			if (CurPeek == EDcDataEntry::Name)
			{
				DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

				FName Value;
				DC_TRY(Ctx.Reader->ReadName(&Value, nullptr));
				DC_TRY(Ctx.Writer->WriteName(Value));
			}
			else if (CurPeek == EDcDataEntry::String)
			{
				DC_TRY(Ctx.Writer->Peek(EDcDataEntry::Name));

				FString Value;
				DC_TRY(Ctx.Reader->ReadString(&Value, nullptr));
				DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
			}
			else
			{
				return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch2))
					<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
			}

			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

			CurPeek = Ctx.Reader->Peek();
		}

		DC_TRY(Ctx.Reader->ReadMapEnd(nullptr));
		DC_TRY(Ctx.Writer->WriteStructEnd(GetStructName(Ctx.TopProperty())));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DcFail(DC_DIAG(DcDDeserialize, DataEntryMismatch))
			<< EDcDataEntry::MapRoot << Next;
	}
}

}	// namespace DcHandlers

