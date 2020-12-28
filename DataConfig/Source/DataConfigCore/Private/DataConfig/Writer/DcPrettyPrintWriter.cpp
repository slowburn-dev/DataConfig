#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "Misc/OutputDevice.h"
#include "DataConfig/Property/DcPropertyUtils.h"

static const FString _PER_INDENT = FString(TEXT("    "));

FDcPrettyPrintWriter::FDcPrettyPrintWriter(FOutputDevice& InOutput)
	: Output(InOutput)
{}

FDcPrettyPrintWriter::~FDcPrettyPrintWriter()
{
	Output.Flush();
}

FDcResult FDcPrettyPrintWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	ReadOut(bOutOk, true);
	return DcOk();	// accepts anything
}

FDcResult FDcPrettyPrintWriter::WriteBool(bool Value)
{
	Output.Logf(TEXT("%s- bool: %s"), *Indent, Value ? TEXT("true") : TEXT("false"));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteName(const FName& Value)
{
	Output.Logf(TEXT("%s- name: \"%s\""), *Indent, *DcPropertyUtils::SafeNameToString(Value));
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
	Output.Logf(TEXT("%s- enum: %s,%s,unsigned: %d, %d"),
		*Indent,
		*DcPropertyUtils::SafeNameToString(Value.Type),
		*DcPropertyUtils::SafeNameToString(Value.Name), 
		Value.bIsUnsigned,
		Value.Signed64
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructRoot(const FDcStructStat& Struct)
{
	Output.Logf(TEXT("%s- struct begin: <%s>"), *Indent, *DcPropertyUtils::SafeNameToString(Struct.Name));
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructEnd(const FDcStructStat& Struct)
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- struct end: <%s>"), *Indent, *DcPropertyUtils::SafeNameToString(Struct.Name));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassRoot(const FDcClassStat& Class)
{
	Output.Logf(TEXT("%s- class begin: <%s>"), *Indent, *DcPropertyUtils::SafeNameToString(Class.Name));
	Indent += _PER_INDENT;
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassEnd(const FDcClassStat& Class)
{
	Indent = Indent.Left(Indent.Len() - _PER_INDENT.Len());
	Output.Logf(TEXT("%s- class end: <%s>"), *Indent, *DcPropertyUtils::SafeNameToString(Class.Name));
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

FDcResult FDcPrettyPrintWriter::WriteSoftObjectReference(const FSoftObjectPath& Value)
{
	Output.Logf(TEXT("%s- soft object reference: %s"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSoftClassReference(const FSoftClassPath& Value)
{
	Output.Logf(TEXT("%s- soft class reference: %s"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInterfaceReference(const FScriptInterface& Value)
{
	Output.Logf(TEXT("%s- interface reference: %s"), *Indent,
		Value.GetObject()
			? *Value.GetObject()->GetName()
			: TEXT("<null>")
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteFieldPath(const FFieldPath& Value)
{
	Output.Logf(TEXT("%s- field path: %s"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteDelegate(const FScriptDelegate& Value)
{
	Output.Logf(TEXT("%s- delegate: %s"), *Indent, *Value.ToString<UObject>());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value)
{
	Output.Logf(TEXT("%s- multicast inline delegate: %s"), *Indent, *Value.ToString<UObject>());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value)
{
	Output.Logf(TEXT("%s- multicast sparse delegate: %s"), *Indent, *Value.ToString<UObject>());
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
	Output.Logf(TEXT("%s- uint8: \"%u\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt16(const uint16& Value)
{
	Output.Logf(TEXT("%s- uint16: \"%u\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt32(const uint32& Value)
{
	Output.Logf(TEXT("%s- uint32: \"%u\""), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt64(const uint64& Value)
{
	Output.Logf(TEXT("%s- uint64: \"%u\""), *Indent, Value);
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

FDcResult FDcPrettyPrintWriter::WriteBlob(const FDcBlobViewData& Value)
{
	Output.Logf(TEXT("%s- blob: 0x%x, size: %d"), *Indent, (UPTRINT)Value.DataPtr, Value.Num);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteNil()
{
	Output.Logf(TEXT("%s- nil"), *Indent);
	return DcOk();
}

