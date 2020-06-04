#include "Reader/Reader.h"

namespace DataConfig
{

FReader::~FReader() {}
EDataEntry FReader::Peek() { return EDataEntry::Ended; }
FResult FReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadBoolFail); }
FResult FReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadNameFail); }
FResult FReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadStringFail); }
FResult FReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadStructFail); }
FResult FReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadStructEndFail); }
FResult FReader::ReadClassRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadClassFail); }
FResult FReader::ReadClassEnd(FName* OutNamePtr, FContextStorage* CtxPtr) {	return Fail(EErrorCode::ReadClassEndFail); }
FResult FReader::ReadMapRoot(FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadMapFail); }
FResult FReader::ReadMapEnd(FContextStorage* CtxPtr) { return Fail(EErrorCode::ReadMapEndFail); }

} // namespace DataConfig






