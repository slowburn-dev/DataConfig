#include "DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DcPropertyPipeHandlers {

FDcResult HandlerScalarDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	DC_TRY(DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	return DcOk();
}

template<EDcDataEntry EntryStart,
	EDcDataEntry EntryEnd,
	FDcResult (FDcReader::*ReadMethodStart)(),
	FDcResult (FDcReader::*ReadMethodEnd)(),
	FDcResult (FDcPropertyWriter::*WriteMethodStart)(),
	FDcResult (FDcPropertyWriter::*WriteMethodEnd)()>
FORCEINLINE static FDcResult _HandlerLinearContainerDeserialize(
	FDcDeserializeContext& Ctx
)
{
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

	return DcOk();
}


FDcResult HandlerArrayDeserialize(FDcDeserializeContext& Ctx)
{
	return _HandlerLinearContainerDeserialize<
		EDcDataEntry::ArrayRoot,
		EDcDataEntry::ArrayEnd,
		&FDcReader::ReadArrayRoot,
		&FDcReader::ReadArrayEnd,
		&FDcPropertyWriter::WriteArrayRoot,
		&FDcPropertyWriter::WriteArrayEnd>(
		Ctx
	);
}

FDcResult HandlerSetDeserialize(FDcDeserializeContext& Ctx)
{
	return _HandlerLinearContainerDeserialize<
		EDcDataEntry::SetRoot,
		EDcDataEntry::SetEnd,
		&FDcReader::ReadSetRoot,
		&FDcReader::ReadSetEnd,
		&FDcPropertyWriter::WriteSetRoot,
		&FDcPropertyWriter::WriteSetEnd>(
		Ctx
	);
}

FDcResult HandlerMapDeserialize(FDcDeserializeContext& Ctx)
{
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

	return DcOk();
}

FDcResult HandlerStructDeserialize(FDcDeserializeContext& Ctx)
{
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

	return DcOk();
}

FDcResult HandlerClassDeserialize(FDcDeserializeContext& Ctx)
{
	FDcClassStat ClassStat;
	DC_TRY(Ctx.Reader->ReadClassRoot(&ClassStat));
	DC_TRY(Ctx.Writer->WriteClassRoot(ClassStat));

	if (ClassStat.Control == FDcClassStat::EControl::ReferenceOrNil)
	{
		EDcDataEntry Next;
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

	return DcOk();
}

} // namespace DcPropertyPipeHandlers


