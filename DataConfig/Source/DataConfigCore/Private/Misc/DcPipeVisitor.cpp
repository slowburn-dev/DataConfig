#include "Misc/DcPipeVisitor.h"
#include "Reader/DcReader.h"
#include "Writer/DcWriter.h"

namespace DataConfig
{

FPipeVisitor::FPipeVisitor(FReader* InReader, FWriter* InWriter)
{
	Reader = InReader;
	Writer = InWriter;
}

FResult FPipeVisitor::PipeVisit()
{
	while (true)
	{
		EDataEntry PeekEntry = Reader->Peek();
		TRY(Writer->Peek(PeekEntry));

		if (PeekEntry == EDataEntry::Nil)
		{
			TRY(Reader->ReadNil(nullptr));
			TRY(Writer->WriteNil());
		}
		else if (PeekEntry == EDataEntry::Bool)
		{
			bool Value;
			TRY(Reader->ReadBool(&Value, nullptr));
			TRY(Writer->WriteBool(Value));
		}
		else if (PeekEntry == EDataEntry::Name)
		{
			FName Value;
			TRY(Reader->ReadName(&Value, nullptr));
			TRY(Writer->WriteName(Value));
		}
		else if (PeekEntry == EDataEntry::String)
		{
			FString Value;
			TRY(Reader->ReadString(&Value, nullptr));
			TRY(Writer->WriteString(Value));
		}
		else if (PeekEntry == EDataEntry::StructRoot)
		{
			FName Name;
			TRY(Reader->ReadStructRoot(&Name, nullptr));
			TRY(Writer->WriteStructRoot(Name));
		}
		else if (PeekEntry == EDataEntry::StructEnd)
		{
			FName Name;
			TRY(Reader->ReadStructEnd(&Name, nullptr));
			TRY(Writer->WriteStructEnd(Name));
		}
		else if (PeekEntry == EDataEntry::MapRoot)
		{
			TRY(Reader->ReadMapRoot(nullptr));
			TRY(Writer->WriteMapRoot());
		}
		else if (PeekEntry == EDataEntry::MapEnd)
		{
			TRY(Reader->ReadMapEnd(nullptr));
			TRY(Writer->WriteMapEnd());
		}
		else if (PeekEntry == EDataEntry::ClassRoot)
		{
			FClassPropertyStat ClassStat;
			TRY(Reader->ReadClassRoot(&ClassStat, nullptr));
			TRY(Writer->WriteClassRoot(ClassStat));
		}
		else if (PeekEntry == EDataEntry::ClassEnd)
		{
			FClassPropertyStat ClassStat;
			TRY(Reader->ReadClassEnd(&ClassStat, nullptr));
			TRY(Writer->WriteClassEnd(ClassStat));
		}
		else if (PeekEntry == EDataEntry::ArrayRoot)
		{
			TRY(Reader->ReadArrayRoot(nullptr));
			TRY(Writer->WriteArrayRoot());
		}
		else if (PeekEntry == EDataEntry::ArrayEnd)
		{
			TRY(Reader->ReadArrayEnd(nullptr));
			TRY(Writer->WriteArrayEnd());
		}
		else if (PeekEntry == EDataEntry::Reference)
		{
			UObject* Value;
			TRY(Reader->ReadReference(&Value, nullptr));
			TRY(Writer->WriteReference(Value));
		}
		else if (PeekEntry == EDataEntry::Ended)
		{
			return Ok();
		}
		else
		{
			checkNoEntry();
		}
	}
}


}	// namespace DataConfig





