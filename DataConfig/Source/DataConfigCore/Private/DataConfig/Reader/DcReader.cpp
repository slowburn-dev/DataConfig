#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"

namespace DataConfig
{

FReader::~FReader() {}
EDataEntry FReader::Peek() { return EDataEntry::Ended; }

FResult FReader::ReadNil(FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadMapRoot(FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadMapEnd(FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadArrayRoot(FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadArrayEnd(FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }
FResult FReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) { return Fail( DCommon::Category, DCommon::NotImplemented ); }

} // namespace DataConfig






