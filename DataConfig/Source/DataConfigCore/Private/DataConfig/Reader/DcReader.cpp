#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"

FDcReader::~FDcReader() {}
EDcDataEntry FDcReader::Peek() { return EDcDataEntry::Ended; }

FDcResult FDcReader::ReadNil(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapRoot(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapEnd(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayRoot(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayEnd(FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }




