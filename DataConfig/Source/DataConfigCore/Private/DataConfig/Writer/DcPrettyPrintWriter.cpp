#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "Misc/OutputDevice.h"

static const FString _PER_INDENT = FString(TEXT("    "));

FDcPrettyPrintWriter::FDcPrettyPrintWriter(FOutputDevice& InOutput)
	: Output(InOutput)
{}

FDcPrettyPrintWriter::~FDcPrettyPrintWriter()
{
	Output.Flush();
}

FDcResult FDcPrettyPrintWriter::WriteNext(EDcDataEntry Next)
{
	return DcOk();	// accepts anything
}

FDcResult FDcPrettyPrintWriter::WriteBool(bool Value)
{
	Output.Logf(TEXT("%s- bool: %d"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteName(const FName& Value)
{
	Output.Logf(TEXT("%s- name: \"%s\""), *Indent, *Value.ToString());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteString(const FString& Value)
{
	Output.Logf(TEXT("%s- str: \"%s\""), *Indent, *Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructRoot(const FName& Name)
{
	Output.Logf(TEXT("%s- struct begin: <%s>"), *Indent, *Name.ToString());
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructEnd(const FName& Name)
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- struct end: <%s>"), *Indent, *Name.ToString());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassRoot(const FDcClassPropertyStat& Class)
{
	Output.Logf(TEXT("%s- class begin: <%s>"), *Indent, *Class.Name.ToString());
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassEnd(const FDcClassPropertyStat& Class)
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- class end: <%s>"), *Indent, *Class.Name.ToString());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMapRoot()
{
	Output.Logf(TEXT("%s- map begin"), *Indent);
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMapEnd()
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- map end"), *Indent);
	return DcOk();
}


FDcResult FDcPrettyPrintWriter::WriteArrayRoot()
{
	Output.Logf(TEXT("%s- array begin"), *Indent);
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteArrayEnd()
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- array end"), *Indent);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteReference(UObject* Value)
{
	check(Value);
	Output.Logf(TEXT("%s- ref: %d"), *Indent, Value->GetUniqueID());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteNil()
{
	Output.Logf(TEXT("%s- nil"), *Indent);
	return DcOk();
}

