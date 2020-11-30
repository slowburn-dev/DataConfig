#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"

FDcWriter::FDcWriter()
{}

FDcWriter::~FDcWriter()
{}

FDcResult FDcWriter::WriteNext(EDcDataEntry) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteNil(){ return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteBool(bool) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteName(const FName&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteString(const FString&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteText(const FText&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteEnum(const FDcEnumData&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructRoot(const FName&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructEnd(const FName&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassRoot(const FDcObjectPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassEnd(const FDcObjectPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSetRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSetEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteObjectReference(const UObject*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassReference(const UClass*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteWeakObjectReference(const FWeakObjectPtr&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteLazyObjectReference(const FLazyObjectPtr&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSoftObjectReference(const FSoftObjectPath&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSoftClassReference(const FSoftClassPath&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInterfaceReference(const FScriptInterface&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteDelegate(const FScriptDelegate&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMulticastSparseDelegate(const FSparseDelegate&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt8(const int8&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt16(const int16&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt32(const int32&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInt64(const int64&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt8(const uint8&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt16(const uint16&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt32(const uint32&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteUInt64(const uint64&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteFloat(const float&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteDouble(const double&) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteBlob(const FDcBlobViewData&) { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcWriter::FormatDiagnostic(FDcDiagnostic& Diag) { /*pass*/ }

