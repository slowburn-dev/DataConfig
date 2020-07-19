#include "DataConfig/Writer/DcWriter.h"

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

FResult FWriter::WriteNil(){ return Fail(EErrorCode::WriteNilFail); }
FResult FWriter::WriteBool(bool Value) { return Fail(EErrorCode::WriteBoolFail); }
FResult FWriter::WriteName(const FName& Value) { return Fail(EErrorCode::WriteNameFail); }
FResult FWriter::WriteString(const FString& Value) { return Fail(EErrorCode::WriteStringFail); }
FResult FWriter::WriteStructRoot(const FName& Name) { return Fail(EErrorCode::WriteStructRootFail); }
FResult FWriter::WriteStructEnd(const FName& Name) { return Fail(EErrorCode::WriteStructEndFail); }
FResult FWriter::WriteClassRoot(const FClassPropertyStat& Class) { return Fail(EErrorCode::WriteClassFail); }
FResult FWriter::WriteClassEnd(const FClassPropertyStat& Class) { return Fail(EErrorCode::WriteClassEndFail); }
FResult FWriter::WriteMapRoot() { return Fail(EErrorCode::WriteMapFail); }
FResult FWriter::WriteMapEnd() { return Fail(EErrorCode::WriteMapEndFail); }
FResult FWriter::WriteArrayRoot() { return Fail(EErrorCode::WriteArrayFail); }
FResult FWriter::WriteArrayEnd() { return Fail(EErrorCode::WriteArrayEndFail); }
FResult FWriter::WriteReference(UObject* Value) { return Fail(EErrorCode::WriteReferenceFail); }

}	// namespace DataConfig


