#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"

FDcPipeVisitor::FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter)
{
	Reader = InReader;
	Writer = InWriter;
}

FDcResult FDcPipeVisitor::PipeVisit()
{
	while (true)
	{
		EDcDataEntry PeekEntry;
		DC_TRY(Reader->PeekRead(&PeekEntry));
		DC_TRY(Writer->PeekWrite(PeekEntry));

		if (PeekEntry == EDcDataEntry::Nil)
		{
			DC_TRY(Reader->ReadNil());
			DC_TRY(Writer->WriteNil());
		}
		else if (PeekEntry == EDcDataEntry::Bool)
		{
			bool Value;
			DC_TRY(Reader->ReadBool(&Value));
			DC_TRY(Writer->WriteBool(Value));
		}
		else if (PeekEntry == EDcDataEntry::Name)
		{
			FName Value;
			DC_TRY(Reader->ReadName(&Value));
			DC_TRY(Writer->WriteName(Value));
		}
		else if (PeekEntry == EDcDataEntry::String)
		{
			FString Value;
			DC_TRY(Reader->ReadString(&Value));
			DC_TRY(Writer->WriteString(Value));
		}
		else if (PeekEntry == EDcDataEntry::StructRoot)
		{
			FName Name;
			DC_TRY(Reader->ReadStructRoot(&Name));
			DC_TRY(Writer->WriteStructRoot(Name));
		}
		else if (PeekEntry == EDcDataEntry::StructEnd)
		{
			FName Name;
			DC_TRY(Reader->ReadStructEnd(&Name));
			DC_TRY(Writer->WriteStructEnd(Name));
		}
		else if (PeekEntry == EDcDataEntry::MapRoot)
		{
			DC_TRY(Reader->ReadMapRoot());
			DC_TRY(Writer->WriteMapRoot());
		}
		else if (PeekEntry == EDcDataEntry::MapEnd)
		{
			DC_TRY(Reader->ReadMapEnd());
			DC_TRY(Writer->WriteMapEnd());
		}
		else if (PeekEntry == EDcDataEntry::ClassRoot)
		{
			FDcClassPropertyStat ClassStat;
			DC_TRY(Reader->ReadClassRoot(&ClassStat));
			DC_TRY(Writer->WriteClassRoot(ClassStat));
		}
		else if (PeekEntry == EDcDataEntry::ClassEnd)
		{
			FDcClassPropertyStat ClassStat;
			DC_TRY(Reader->ReadClassEnd(&ClassStat));
			DC_TRY(Writer->WriteClassEnd(ClassStat));
		}
		else if (PeekEntry == EDcDataEntry::ArrayRoot)
		{
			DC_TRY(Reader->ReadArrayRoot());
			DC_TRY(Writer->WriteArrayRoot());
		}
		else if (PeekEntry == EDcDataEntry::ArrayEnd)
		{
			DC_TRY(Reader->ReadArrayEnd());
			DC_TRY(Writer->WriteArrayEnd());
		}
		else if (PeekEntry == EDcDataEntry::Reference)
		{
			UObject* Value;
			DC_TRY(Reader->ReadReference(&Value));
			DC_TRY(Writer->WriteReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::Ended)
		{
			return DcOk();
		}
		else
		{
			checkNoEntry();
		}
	}
}

