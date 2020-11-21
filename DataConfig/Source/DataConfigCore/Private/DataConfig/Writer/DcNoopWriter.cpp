#include "DataConfig/Writer/DcNoopWriter.h"

FDcNoopWriter::FDcNoopWriter() {}
FDcNoopWriter::~FDcNoopWriter() {}

FDcResult FDcNoopWriter::WriteNext(EDcDataEntry) { return DcOk(); }
FDcResult FDcNoopWriter::WriteNil() { return DcOk(); }
FDcResult FDcNoopWriter::WriteBool(bool) { return DcOk(); }
FDcResult FDcNoopWriter::WriteName(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteString(const FString&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteText(const FText& Value) { return DcOk(); }
FDcResult FDcNoopWriter::WriteEnum(const FDcEnumData& Value) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructRoot(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructEnd(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassRoot(const FDcClassPropertyStat&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassEnd(const FDcClassPropertyStat&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteReference(UObject*) { return DcOk(); }
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

