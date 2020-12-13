#include "DataConfig/Writer/DcNoopWriter.h"

FDcNoopWriter::FDcNoopWriter() {}
FDcNoopWriter::~FDcNoopWriter() {}

FDcResult FDcNoopWriter::PeekWrite(EDcDataEntry, bool* bOutOk)
{ 
	ReadOut(bOutOk, true);
	return DcOk(); 
}
FDcResult FDcNoopWriter::WriteNil() { return DcOk(); }
FDcResult FDcNoopWriter::WriteBool(bool) { return DcOk(); }
FDcResult FDcNoopWriter::WriteName(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteString(const FString&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteText(const FText&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteEnum(const FDcEnumData&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructRoot(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructEnd(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassRoot(const FDcObjectPropertyStat&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassEnd(const FDcObjectPropertyStat&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteObjectReference(const UObject*) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassReference(const UClass*) { return DcOk(); }
FDcResult FDcNoopWriter::WriteWeakObjectReference(const FWeakObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteLazyObjectReference(const FLazyObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteSoftObjectReference(const FSoftObjectPath&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteSoftClassReference(const FSoftClassPath&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInterfaceReference(const FScriptInterface&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteFieldPath(const FFieldPath& Value) { return DcOk(); }
FDcResult FDcNoopWriter::WriteDelegate(const FScriptDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMulticastSparseDelegate(const FSparseDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt8(const int8&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt16(const int16&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt32(const int32&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt64(const int64&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt8(const uint8&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt16(const uint16&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt32(const uint32&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt64(const uint64&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteFloat(const float&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteDouble(const double&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteBlob(const FDcBlobViewData&) { return DcOk(); }

