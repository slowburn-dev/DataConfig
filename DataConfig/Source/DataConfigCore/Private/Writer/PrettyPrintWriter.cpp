#include "Writer/PrettyPrintWriter.h"
#include "Misc/OutputDevice.h"

namespace DataConfig
{

static const FString PER_INDENT = FString(TEXT("    "));

FPrettyPrintWriter::FPrettyPrintWriter(FOutputDevice& InOutput)
	: Output(InOutput)
{}

FPrettyPrintWriter::~FPrettyPrintWriter()
{
	Output.Flush();
}

FResult FPrettyPrintWriter::Peek(EDataEntry Next)
{
	return Ok();	// accepts anything
}

FResult FPrettyPrintWriter::WriteBool(bool Value)
{
	Output.Logf(TEXT("%s- bool: %d"), *Indent, Value);
	return Ok();
}

FResult FPrettyPrintWriter::WriteName(const FName& Value)
{
	Output.Logf(TEXT("%s- name: \"%s\""), *Indent, *Value.ToString());
	return Ok();
}

FResult FPrettyPrintWriter::WriteString(const FString& Value)
{
	Output.Logf(TEXT("%s- str: \"%s\""), *Indent, *Value);
	return Ok();
}

FResult FPrettyPrintWriter::WriteStructRoot(const FName& Name)
{
	Output.Logf(TEXT("%s- struct begin: <%s>"), *Indent, *Name.ToString());
	Indent += PER_INDENT;
	return Ok();
}

FResult FPrettyPrintWriter::WriteStructEnd(const FName& Name)
{
	Indent = Indent.Left(Indent.Len() - PER_INDENT.Len());
	Output.Logf(TEXT("%s- struct end: <%s>"), *Indent, *Name.ToString());
	return Ok();
}


}	// namespace DataConfig
