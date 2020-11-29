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
FDcResult FDcWriter::WriteText(const FText& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteEnum(const FDcEnumData& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructRoot(const FName& Name) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteStructEnd(const FName& Name) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassRoot(const FDcObjectPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassEnd(const FDcObjectPropertyStat& Class) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteMapEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteArrayEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSetRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSetEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteObjectReference(const UObject* Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteClassReference(const UClass* Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSoftObjectReference(const FSoftObjectPath& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteSoftClassReference(const FSoftClassPath& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcWriter::WriteInterfaceReference(const FScriptInterface& Value) { return DC_FAIL(DcDCommon, NotImplemented); }
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

void FDcWriter::FormatDiagnostic(FDcDiagnostic& Diag) { /*pass*/ }

