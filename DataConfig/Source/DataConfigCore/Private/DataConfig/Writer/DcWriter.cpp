#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"

FDcWriter::FDcWriter()
{}

FDcWriter::~FDcWriter()
{}

FDcResult FDcWriter::Peek(EDcDataEntry Next) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteNil(){ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteBool(bool Value) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteName(const FName& Value) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteString(const FString& Value) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteStructRoot(const FName& Name) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteStructEnd(const FName& Name) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteClassRoot(const FDcClassPropertyStat& Class) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteClassEnd(const FDcClassPropertyStat& Class) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteMapRoot() { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteMapEnd() { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteArrayRoot() { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteArrayEnd() { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcWriter::WriteReference(UObject* Value) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }


