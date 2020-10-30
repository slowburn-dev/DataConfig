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
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->ReadNext(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass = Ctx.Writer->WriteNext(EDcDataEntry::StructRoot).Ok();
	if (!bRootPeekPass
		|| !bWritePass)
	{
		return DcOkWithCanNotProcess(OutRet);
	}

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		DC_TRY(Ctx.Writer->WriteStructRoot(GetStructName(Ctx.TopProperty())));

		EDcDataEntry CurPeek;
		DC_TRY(Ctx.Reader->ReadNext(&CurPeek));
		while (CurPeek != EDcDataEntry::MapEnd)
		{
			if (CurPeek == EDcDataEntry::Name)
			{
				DC_TRY(Ctx.Writer->WriteNext(EDcDataEntry::Name));

				FName Value;
				DC_TRY(Ctx.Reader->ReadName(&Value));
				DC_TRY(Ctx.Writer->WriteName(Value));
			}
			else if (CurPeek == EDcDataEntry::String)
			{
				DC_TRY(Ctx.Writer->WriteNext(EDcDataEntry::Name));

				FString Value;
				DC_TRY(Ctx.Reader->ReadString(&Value));
				DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
			}
			else
			{
				return DC_FAIL(DcDDeserialize, DataEntryMismatch2)
					<< EDcDataEntry::Name << EDcDataEntry::String << CurPeek;
			}

			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));

			DC_TRY(Ctx.Reader->ReadNext(&CurPeek));
		}

		DC_TRY(Ctx.Reader->ReadMapEnd());
		DC_TRY(Ctx.Writer->WriteStructEnd(GetStructName(Ctx.TopProperty())));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::MapRoot << Next;
	}
}

}	// namespace DcHandlers

