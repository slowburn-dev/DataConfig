#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/DcEnv.h"

namespace DataConfig
{

FWriter::FWriter()
{}

FWriter::~FWriter()
{}

DataConfig::FResult FWriter::Peek(EDataEntry Next)
{
	return Fail(EErrorCode::UnknownError);
}

FResult FWriter::WriteNil(){ return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteBool(bool Value) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteName(const FName& Value) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteString(const FString& Value) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteStructRoot(const FName& Name) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteStructEnd(const FName& Name) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteClassRoot(const FClassPropertyStat& Class) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteClassEnd(const FClassPropertyStat& Class) { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteMapRoot() { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteMapEnd() { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteArrayRoot() { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteArrayEnd() { return Fail(DCommon::Category, DCommon::NotImplemented); }
FResult FWriter::WriteReference(UObject* Value) { return Fail(DCommon::Category, DCommon::NotImplemented); }

}	// namespace DataConfig


