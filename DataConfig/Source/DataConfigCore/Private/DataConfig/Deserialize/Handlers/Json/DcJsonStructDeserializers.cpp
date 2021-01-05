#include "DataConfig/Deserialize/Handlers/Json/DcJsonStructDeserializers.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"

namespace DcJsonHandlers {

FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!bRootPeekPass
		|| !bWritePass)
	{
		return DcOkWithFallThrough(OutRet);
	}

	if (Next == EDcDataEntry::MapRoot)
	{
		DC_TRY(Ctx.Reader->ReadMapRoot());
		DC_TRY(Ctx.Writer->WriteStructRoot(FDcStructStat()));

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

		DC_TRY(Ctx.Writer->WriteStructEnd(FDcStructStat{}));

		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::MapRoot << Next;
	}
}

}	// namespace DcJsonHandlers

