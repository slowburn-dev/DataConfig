#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"

FDcReader::~FDcReader() {}
EDcDataEntry FDcReader::Peek() { return EDcDataEntry::Ended; }

FDcResult FDcReader::ReadNil()
{ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadBool(bool* OutPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadName(FName* OutPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadString(FString* OutPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructRoot(FName* OutNamePtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadStructEnd(FName* OutNamePtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapRoot()
{ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadMapEnd()
{ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayRoot()
{ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadArrayEnd()
{ return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }
FDcResult FDcReader::ReadReference(UObject** OutPtr) { return DcFail(DC_DIAG(DcDCommon, NotImplemented)); }




