#include "DataConfig/Deserialize/Handlers/DcContainerDeserializers.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcHandlers {


FDcResult DATACONFIGCORE_API HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	//	TOOO [DESERIALIZEUTILS] share code between deserializers, this happnes migrate to 4.25
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::ArrayRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ArrayRoot, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithCanNotProcess(OutRet);

	check(Next == EDcDataEntry::ArrayRoot);

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Writer->WriteArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadArrayEnd());
	DC_TRY(Ctx.Writer->WriteArrayEnd());

	return DcOkWithProcessed(OutRet);
}



}	// namespace DcHandlers

