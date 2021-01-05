#include "DataConfig/Deserialize/Handlers/Json/DcJsonContainerDeserializers.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"

namespace DcJsonHandlers {

template<EDcDataEntry EntryStart, typename TMethodStart, typename TMethodEnd>
FORCEINLINE static FDcResult _HandlerLinearContainerDeserialize(
	FDcDeserializeContext& Ctx,
	EDcDeserializeResult& OutRet,
	TMethodStart MethodStart,
	TMethodEnd MethodEnd)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::ArrayRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EntryStart, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	check(Next == EDcDataEntry::ArrayRoot);

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY((Ctx.Writer->*MethodStart)());

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
	DC_TRY((Ctx.Writer->*MethodEnd)());

	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return _HandlerLinearContainerDeserialize<EDcDataEntry::ArrayRoot>(Ctx, OutRet, &FDcWriter::WriteArrayRoot, &FDcWriter::WriteArrayEnd);
}

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return _HandlerLinearContainerDeserialize<EDcDataEntry::SetRoot>(Ctx, OutRet, &FDcWriter::WriteSetRoot, &FDcWriter::WriteSetEnd);
}

FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::MapRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::MapRoot, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	check(Next == EDcDataEntry::MapRoot);

	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		//	allow only string key
		FString Key;
		DC_TRY(Ctx.Reader->ReadString(&Key));
		DC_TRY(Ctx.Writer->WriteString(Key));

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOkWithProcessed(OutRet);
}

}	// namespace DcJsonHandlers

