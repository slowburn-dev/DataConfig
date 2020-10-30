#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"

FDcReader::~FDcReader() {}

bool FDcReader::Coercion(EDcDataEntry ToEntry)
{
	return false;
}

FDcResult FDcReader::ReadNext(EDcDataEntry* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadNil() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadBool(bool* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadName(FName* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadString(FString* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadStructRoot(FName* OutNamePtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadStructEnd(FName* OutNamePtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadClassRoot(FDcClassPropertyStat* OutClassPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadClassEnd(FDcClassPropertyStat* OutClassPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMapRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMapEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadArrayRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadArrayEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadReference(UObject** OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt8(int8* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt16(int16* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt32(int32* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt64(int64* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt8(uint8* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt16(uint16* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt32(uint32* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt64(uint64* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }

