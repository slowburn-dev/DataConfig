#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "Misc/OutputDevice.h"
#include "DataConfig/Misc/DcTypeUtils.h"

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
	Output.Logf(TEXT("%s- bool: %s"), *Indent, Value ? TEXT("true") : TEXT("false"));
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

FDcResult FDcPrettyPrintWriter::WriteText(const FText& Value)
{
	Output.Logf(TEXT("%s- text: \"%s\""), *Indent, *Value.ToString());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteEnum(const FDcEnumData& Value)
{
	Output.Logf(TEXT("%s- enum: %s,%s,%d"), *Indent, *Value.Type.ToString(), *Value.Name.ToString(), Value.Value);
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

FDcResult FDcPrettyPrintWriter::WriteClassRoot(const FDcObjectPropertyStat& Class)
{
	Output.Logf(TEXT("%s- class begin: <%s>"), *Indent, *Class.Name.ToString());
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassEnd(const FDcObjectPropertyStat& Class)
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

FDcResult FDcPrettyPrintWriter::WriteSetRoot()
{
	Output.Logf(TEXT("%s- set begin"), *Indent);
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSetEnd()
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- set end"), *Indent);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteObjectReference(const UObject* Value)
{
	check(Value);	// note that this is guarenteed to be non-null 
	Output.Logf(TEXT("%s- ref: %d"), *Indent, Value->GetUniqueID());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassReference(const UClass* Value)
{
	Output.Logf(TEXT("%s- class: \"%s\""), *Indent, Value == nullptr 
		? TEXT("<null>") 
		: *Value->GetName()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value)
{
	struct FView
	{
		int32		ObjectIndex;
		int32		ObjectSerialNumber;
	};
	static_assert(DcTypeUtils::TIsSameSize<FView, FWeakObjectPtr>::Value, "should be same size");

	const FView& View = (const FView&)Value;
	Output.Logf(TEXT("%s- weak object reference: index: %d, gen: %d"), *Indent, 
		View.ObjectIndex,
		View.ObjectSerialNumber
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value)
{
	Output.Logf(TEXT("%s- lazy object reference: %s"), *Indent,
		*Value.GetUniqueID().ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSoftObjectReference(const FSoftObjectPtr& Value)
{
	Output.Logf(TEXT("%s- soft object reference: %s"), *Indent,
		*Value.GetUniqueID().ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt8(const int8& Value)
{
	Output.Logf(TEXT("%s- int8: \"%d\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt16(const int16& Value)
{
	Output.Logf(TEXT("%s- int16: \"%d\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt32(const int32& Value)
{
	Output.Logf(TEXT("%s- int32: \"%d\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt64(const int64& Value)
{
	Output.Logf(TEXT("%s- int64: \"%d\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt8(const uint8& Value)
{
	Output.Logf(TEXT("%s- uint8: \"%o\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt16(const uint16& Value)
{
	Output.Logf(TEXT("%s- uint16: \"%o\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt32(const uint32& Value)
{
	Output.Logf(TEXT("%s- uint32: \"%o\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt64(const uint64& Value)
{
	Output.Logf(TEXT("%s- uint64: \"%o\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteFloat(const float& Value)
{
	Output.Logf(TEXT("%s- float: \"%f\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteDouble(const double& Value)
{
	Output.Logf(TEXT("%s- double: \"%f\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteNil()
{
	Output.Logf(TEXT("%s- nil"), *Indent);
	return DcOk();
}

