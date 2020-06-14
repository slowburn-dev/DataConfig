#include "Writer/DcPrettyPrintWriter.h"
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

DataConfig::FResult FPrettyPrintWriter::WriteClassRoot(const FClassPropertyStat& Class)
{
	Output.Logf(TEXT("%s- class begin: <%s>"), *Indent, *Class.Name.ToString());
	Indent += PER_INDENT;
	return Ok();
}

DataConfig::FResult FPrettyPrintWriter::WriteClassEnd(const FClassPropertyStat& Class)
{
	Indent = Indent.Left(Indent.Len() - PER_INDENT.Len());
	Output.Logf(TEXT("%s- class end: <%s>"), *Indent, *Class.Name.ToString());
	return Ok();
}

FResult FPrettyPrintWriter::WriteMapRoot()
{
	Output.Logf(TEXT("%s- map begin"), *Indent);
	Indent += PER_INDENT;
	return Ok();
}

FResult FPrettyPrintWriter::WriteMapEnd()
{
	Indent = Indent.Left(Indent.Len() - PER_INDENT.Len());
	Output.Logf(TEXT("%s- map end"), *Indent);
	return Ok();
}


FResult FPrettyPrintWriter::WriteArrayRoot()
{
	Output.Logf(TEXT("%s- array begin"), *Indent);
	Indent += PER_INDENT;
	return Ok();
}

FResult FPrettyPrintWriter::WriteArrayEnd()
{
	Indent = Indent.Left(Indent.Len() - PER_INDENT.Len());
	Output.Logf(TEXT("%s- array end"), *Indent);
	return Ok();
}

DataConfig::FResult FPrettyPrintWriter::WriteReference(UObject* Value)
{
	check(Value);
	Output.Logf(TEXT("%s- ref: %d"), *Indent, Value->GetUniqueID());
	return Ok();
}

FResult FPrettyPrintWriter::WriteNil()
{
	Output.Logf(TEXT("%s- nil"), *Indent);
	return Ok();
}


}	// namespace DataConfig
