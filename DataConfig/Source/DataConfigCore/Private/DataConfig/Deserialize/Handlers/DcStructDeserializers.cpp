#include "DataConfig/Deserialize/Handlers/DcStructDeserializers.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcHandlers {

static FName GetStructName(FFieldVariant& Property)
{
	if (!Property.IsValid())
	{
		return FName();
	}
	if (Property.IsA<FStructProperty>())
	{
		return CastFieldChecked<FStructProperty>(Property.ToFieldUnsafe()).GetFName();
	}
	else if (Property.IsA<UScriptStruct>())
	{
		return CastChecked<UScriptStruct>(Property.ToUObjectUnsafe().GetFName());
	}
	else
	{
		return FName();
	}
}

FDcResult DATACONFIGCORE_API HandlerStructRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

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
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));

			if (CurPeek == EDcDataEntry::MapEnd)
			{
				DC_TRY(Ctx.Reader->ReadMapEnd());
				break;
			}
			else if (CurPeek == EDcDataEntry::String)
			{
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
		}

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

