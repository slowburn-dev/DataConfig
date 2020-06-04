#include "Adhocs.h"

using namespace DataConfig;

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

		if (PeekEntry == EDataEntry::Bool)
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
			FName Name;
			TRY(Reader->ReadClassRoot(&Name, nullptr));
			TRY(Writer->WriteClassRoot(Name));
		}
		else if (PeekEntry == EDataEntry::ClassEnd)
		{
			FName Name;
			TRY(Reader->ReadClassEnd(&Name, nullptr));
			TRY(Writer->WriteClassEnd(Name));
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



