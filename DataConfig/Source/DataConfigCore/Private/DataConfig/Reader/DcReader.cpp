#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"

FDcReader::~FDcReader() {}
EDataEntry FDcReader::Peek() { return EDataEntry::Ended; }

FDcResult FDcReader::ReadNil(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapRoot(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapEnd(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayRoot(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayEnd(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }
FDcResult FDcReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DCommon, NotImplemented)); }




