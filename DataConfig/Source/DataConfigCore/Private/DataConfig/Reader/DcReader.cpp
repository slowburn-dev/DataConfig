#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"

FDcReader::~FDcReader() {}

FDcResult FDcReader::Coercion(EDcDataEntry ToEntry, bool* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::PeekRead(EDcDataEntry*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadNone() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadBool(bool*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadName(FName*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadString(FString*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadText(FText*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadEnum(FDcEnumData*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadStructRootAccess(FDcStructAccess& Access) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadStructEndAccess(FDcStructAccess& Access) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadClassRootAccess(FDcClassAccess& Access) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadClassEndAccess(FDcClassAccess& Access) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMapRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMapEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadArrayRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadArrayEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadSetRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadSetEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadOptionalRoot() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadOptionalEnd() { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadObjectReference(UObject**) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadClassReference(UClass**) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadWeakObjectReference(FWeakObjectPtr*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadLazyObjectReference(FLazyObjectPtr*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadSoftObjectReference(FSoftObjectPtr*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadSoftClassReference(FSoftObjectPtr*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInterfaceReference(FScriptInterface*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadFieldPath(FFieldPath* OutPtr) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadDelegate(FScriptDelegate*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMulticastInlineDelegate(FMulticastScriptDelegate*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadMulticastSparseDelegate(FMulticastScriptDelegate*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt8(int8*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt16(int16*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt32(int32*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadInt64(int64*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt8(uint8*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt16(uint16*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt32(uint32*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadUInt64(uint64*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadFloat(float*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadDouble(double*) { return DC_FAIL(DcDCommon, NotImplemented); }
FDcResult FDcReader::ReadBlob(FDcBlobViewData*) { return DC_FAIL(DcDCommon, NotImplemented); }

void FDcReader::FormatDiagnostic(FDcDiagnostic& Diag) { /*pass*/ }

FName FDcReader::ClassId() { return FName(TEXT("BaseDcReader")); }
FName FDcReader::GetId() { return ClassId(); }


