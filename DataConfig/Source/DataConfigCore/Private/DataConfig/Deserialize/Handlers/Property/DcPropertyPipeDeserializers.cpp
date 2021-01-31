#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DcPropertyPipeHandlers {

FDcResult HandlerScalarDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	DC_TRY(DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	return DcOkWithProcessed(OutRet);
}

template<EDcDataEntry EntryStart,
	EDcDataEntry EntryEnd,
	FDcResult (FDcReader::*ReadMethodStart)(),
	FDcResult (FDcReader::*ReadMethodEnd)(),
	FDcResult (FDcPropertyWriter::*WriteMethodStart)(),
	FDcResult (FDcPropertyWriter::*WriteMethodEnd)()>
FORCEINLINE static FDcResult _HandlerLinearContainerDeserialize(
	FDcDeserializeContext& Ctx,
	EDcDeserializeResult& OutRet
)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EntryStart;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EntryStart, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	DC_TRY((Ctx.Reader->*ReadMethodStart)());
	DC_TRY((Ctx.Writer->*WriteMethodStart)());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EntryEnd)
			break;

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY((Ctx.Reader->*ReadMethodEnd)());
	DC_TRY((Ctx.Writer->*WriteMethodEnd)());

	return DcOkWithProcessed(OutRet);
}


FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return _HandlerLinearContainerDeserialize<
		EDcDataEntry::ArrayRoot,
		EDcDataEntry::ArrayEnd,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteArrayRoot,
		&FDcPropertyWriter::WriteArrayEnd>(
		Ctx,
		OutRet
	);
}

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return _HandlerLinearContainerDeserialize<
		EDcDataEntry::SetRoot,
		EDcDataEntry::SetEnd,
		&FDcReader::ReadSetRoot,
		&FDcReader::ReadSetEnd,
		&FDcPropertyWriter::WriteSetRoot,
		&FDcPropertyWriter::WriteSetEnd>(
		Ctx,
		OutRet
	);
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

	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		{
			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
		}

		{
			FDcScopedProperty ScopedValueProperty(Ctx);
			DC_TRY(ScopedValueProperty.PushProperty());
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
		}
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteMapEnd());

	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::StructRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FDcStructStat StructStat;
	DC_TRY(Ctx.Reader->ReadStructRoot(&StructStat));
	DC_TRY(Ctx.Writer->WriteStructRoot(StructStat));

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::StructEnd)
			break;

		FName FieldName;
		DC_TRY(Ctx.Reader->ReadName(&FieldName));
		DC_TRY(Ctx.Writer->WriteName(FieldName));

		FDcScopedProperty ScopedValueProperty(Ctx);
		DC_TRY(ScopedValueProperty.PushProperty());
		DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadStructEnd(&StructStat));
	DC_TRY(Ctx.Writer->WriteStructEnd(StructStat));

	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerClassDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bRootPeekPass = Next == EDcDataEntry::ClassRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::ClassRoot, &bWritePass));

	if (!(bRootPeekPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FDcClassStat ClassStat;
	DC_TRY(Ctx.Reader->ReadClassRoot(&ClassStat));
	DC_TRY(Ctx.Writer->WriteClassRoot(ClassStat));

	if (ClassStat.Control == FDcClassStat::EControl::ReferenceOrNil)
	{
		DC_TRY(Ctx.Reader->PeekRead(&Next));
		DC_TRY(DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	}
	else
	{
		EDcDataEntry CurPeek;
		while (true)
		{
			DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
			if (CurPeek == EDcDataEntry::ClassEnd)
				break;

			FName FieldName;
			DC_TRY(Ctx.Reader->ReadName(&FieldName));
			DC_TRY(Ctx.Writer->WriteName(FieldName));

			{
				FDcScopedProperty ScopedValueProperty(Ctx);
				DC_TRY(ScopedValueProperty.PushProperty());
				DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
			}
		}
	}

	DC_TRY(Ctx.Reader->ReadClassEnd(&ClassStat));
	DC_TRY(Ctx.Writer->WriteClassEnd(ClassStat));

	return DcOkWithProcessed(OutRet);
}

} // namespace DcPropertyPipeHandlers


