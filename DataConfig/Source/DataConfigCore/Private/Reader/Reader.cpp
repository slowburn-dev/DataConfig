#include "Reader/Reader.h"

namespace DataConfig
{

FReader::~FReader() {}
EDataEntry FReader::Peek() { return EDataEntry::Ended; }
FResult FReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ExpectBoolFail); }
FResult FReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ExpectNameFail); }
FResult FReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ExpectStringFail); }
FResult FReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ExpectStructFail); }
FResult FReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail(EErrorCode::ExpectStructEndFail); }

} // namespace DataConfig






