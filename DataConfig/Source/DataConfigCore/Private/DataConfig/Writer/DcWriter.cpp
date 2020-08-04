#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"

namespace DataConfig
{

FWriter::FWriter()
{}

FWriter::~FWriter()
{}

FResult FWriter::Peek(EDataEntry Next) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteNil(){ return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteBool(bool Value) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteName(const FName& Value) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteString(const FString& Value) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteStructRoot(const FName& Name) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteStructEnd(const FName& Name) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteClassRoot(const FClassPropertyStat& Class) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteClassEnd(const FClassPropertyStat& Class) { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteMapRoot() { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteMapEnd() { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteArrayRoot() { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteArrayEnd() { return Fail(DIAG(DCommon, NotImplemented)); }
FResult FWriter::WriteReference(UObject* Value) { return Fail(DIAG(DCommon, NotImplemented)); }

}	// namespace DataConfig


