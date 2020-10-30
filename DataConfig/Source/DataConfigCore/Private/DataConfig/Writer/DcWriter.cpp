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


