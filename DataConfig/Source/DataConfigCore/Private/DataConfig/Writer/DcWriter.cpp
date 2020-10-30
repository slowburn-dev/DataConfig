#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"

FDcWriter::FDcWriter()
{}

FDcWriter::~FDcWriter()
{}

FDcResult FDcWriter::WriteNext(EDcDataEntry Next) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteNil(){ return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteBool(bool Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteName(const FName& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteString(const FString& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructRoot(const FName& Name) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructEnd(const FName& Name) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassRoot(const FDcClassPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassEnd(const FDcClassPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteReference(UObject* Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt8(const int8& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt16(const int16& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt32(const int32& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt64(const int64& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt8(const uint8& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt16(const uint16& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt32(const uint32& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt64(const uint64& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteFloat(const float& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteDouble(const double& Value) { return DC_FAIL(DcDCommon, NotImplemented); }

