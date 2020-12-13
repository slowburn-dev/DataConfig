#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

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
		DC_TRY(Reader->PeekRead(&PeekEntry));

		bool bWriteOK;
		DC_TRY(Writer->PeekWrite(PeekEntry, &bWriteOK));
		if (!bWriteOK)
			return DC_FAIL(DcDReadWrite, PipeReadWriteMismatch) << PeekEntry;

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
			FDcObjectPropertyStat ClassStat;
			DC_TRY(Reader->ReadClassRoot(&ClassStat));
			DC_TRY(Writer->WriteClassRoot(ClassStat));
		}
		else if (PeekEntry == EDcDataEntry::ClassEnd)
		{
			FDcObjectPropertyStat ClassStat;
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
		else if (PeekEntry == EDcDataEntry::ObjectReference)
		{
			UObject* Value;
			DC_TRY(Reader->ReadObjectReference(&Value));
			DC_TRY(Writer->WriteObjectReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::ClassReference)
		{
			UClass* Value;
			DC_TRY(Reader->ReadClassReference(&Value));
			DC_TRY(Writer->WriteClassReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::WeakObjectReference)
		{
			FWeakObjectPtr Value;
			DC_TRY(Reader->ReadWeakObjectReference(&Value));
			DC_TRY(Writer->WriteWeakObjectReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::LazyObjectReference)
		{
			FLazyObjectPtr Value;
			DC_TRY(Reader->ReadLazyObjectReference(&Value));
			DC_TRY(Writer->WriteLazyObjectReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::SoftObjectReference)
		{
			FSoftObjectPath Value;
			DC_TRY(Reader->ReadSoftObjectReference(&Value));
			DC_TRY(Writer->WriteSoftObjectReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::SoftClassReference)
		{
			FSoftClassPath Value;
			DC_TRY(Reader->ReadSoftClassReference(&Value));
			DC_TRY(Writer->WriteSoftClassReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::InterfaceReference)
		{
			FScriptInterface Value;
			DC_TRY(Reader->ReadInterfaceReference(&Value));
			DC_TRY(Writer->WriteInterfaceReference(Value));
		}
		else if (PeekEntry == EDcDataEntry::FieldPath)
		{
			FFieldPath Value;
			DC_TRY(Reader->ReadFieldPath(&Value));
			DC_TRY(Writer->WriteFieldPath(Value));
		}
		else if (PeekEntry == EDcDataEntry::Delegate)
		{
			FScriptDelegate Value;
			DC_TRY(Reader->ReadDelegate(&Value));
			DC_TRY(Writer->WriteDelegate(Value));
		}
		else if (PeekEntry == EDcDataEntry::MulticastInlineDelegate)
		{
			FMulticastScriptDelegate Value;
			DC_TRY(Reader->ReadMulticastInlineDelegate(&Value));
			DC_TRY(Writer->WriteMulticastInlineDelegate(Value));
		}
		else if (PeekEntry == EDcDataEntry::MulticastSparseDelegate)
		{
			FSparseDelegate Value;
			DC_TRY(Reader->ReadMulticastSparseDelegate(&Value));
			DC_TRY(Writer->WriteMulticastSparseDelegate(Value));
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

