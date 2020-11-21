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
		PreVisit.ExecuteIfBound(this);

		EDcDataEntry PeekEntry;
		DC_TRY(Reader->ReadNext(&PeekEntry));
		DC_TRY(Writer->WriteNext(PeekEntry));

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
		else if (PeekEntry == EDcDataEntry::Text)
		{
			FText Value;
			DC_TRY(Reader->ReadText(&Value));
			DC_TRY(Writer->WriteText(Value));
		}
		else if (PeekEntry == EDcDataEntry::Enum)
		{
			FDcEnumData Value;
			DC_TRY(Reader->ReadEnum(&Value));
			DC_TRY(Writer->WriteEnum(Value));
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
		else if (PeekEntry == EDcDataEntry::SetRoot)
		{
			DC_TRY(Reader->ReadSetRoot());
			DC_TRY(Writer->WriteSetRoot());
		}
		else if (PeekEntry == EDcDataEntry::SetEnd)
		{
			DC_TRY(Reader->ReadSetEnd());
			DC_TRY(Writer->WriteSetEnd());
		}
		else if (PeekEntry == EDcDataEntry::Reference)
		{
			UObject* Value;
			DC_TRY(Reader->ReadReference(&Value));
			DC_TRY(Writer->WriteReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::Int8)
		{
			int8 Value;
			DC_TRY(Reader->ReadInt8(&Value));
			DC_TRY(Writer->WriteInt8(Value));
		}
		else if (PeekEntry == EDcDataEntry::Int16)
		{
			int16 Value;
			DC_TRY(Reader->ReadInt16(&Value));
			DC_TRY(Writer->WriteInt16(Value));
		}
		else if (PeekEntry == EDcDataEntry::Int32)
		{
			int32 Value;
			DC_TRY(Reader->ReadInt32(&Value));
			DC_TRY(Writer->WriteInt32(Value));
		}
		else if (PeekEntry == EDcDataEntry::Int64)
		{
			int64 Value;
			DC_TRY(Reader->ReadInt64(&Value));
			DC_TRY(Writer->WriteInt64(Value));
		}
		else if (PeekEntry == EDcDataEntry::UInt8)
		{
			uint8 Value;
			DC_TRY(Reader->ReadUInt8(&Value));
			DC_TRY(Writer->WriteUInt8(Value));
		}
		else if (PeekEntry == EDcDataEntry::UInt16)
		{
			uint16 Value;
			DC_TRY(Reader->ReadUInt16(&Value));
			DC_TRY(Writer->WriteUInt16(Value));
		}
		else if (PeekEntry == EDcDataEntry::UInt32)
		{
			uint32 Value;
			DC_TRY(Reader->ReadUInt32(&Value));
			DC_TRY(Writer->WriteUInt32(Value));
		}
		else if (PeekEntry == EDcDataEntry::UInt64)
		{
			uint64 Value;
			DC_TRY(Reader->ReadUInt64(&Value));
			DC_TRY(Writer->WriteUInt64(Value));
		}
		else if (PeekEntry == EDcDataEntry::Float)
		{
			float Value;
			DC_TRY(Reader->ReadFloat(&Value));
			DC_TRY(Writer->WriteFloat(Value));
		}
		else if (PeekEntry == EDcDataEntry::Double)
		{
			double Value;
			DC_TRY(Reader->ReadDouble(&Value));
			DC_TRY(Writer->WriteDouble(Value));
		}
		else if (PeekEntry == EDcDataEntry::Ended)
		{
			return DcOk();
		}
		else
		{
			checkNoEntry();
		}

		PostVisit.ExecuteIfBound(this);
	}
}

