#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

FDcResult FDcNoopWriter::PeekWrite(EDcDataEntry, bool* bOutOk)
{ 
	ReadOut(bOutOk, true);
	return DcOk(); 
}
FDcResult FDcNoopWriter::WriteNone() { return DcOk(); }
FDcResult FDcNoopWriter::WriteBool(bool) { return DcOk(); }
FDcResult FDcNoopWriter::WriteName(const FName&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteString(const FString&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteText(const FText&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteEnum(const FDcEnumData&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructRootAccess(FDcStructAccess& Access) { return DcOk(); }
FDcResult FDcNoopWriter::WriteStructEndAccess(FDcStructAccess& Access) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassRootAccess(FDcClassAccess& Access) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassEndAccess(FDcClassAccess& Access) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteMapEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteArrayEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteSetEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteOptionalRoot() { return DcOk(); }
FDcResult FDcNoopWriter::WriteOptionalEnd() { return DcOk(); }
FDcResult FDcNoopWriter::WriteObjectReference(const UObject*) { return DcOk(); }
FDcResult FDcNoopWriter::WriteClassReference(const UClass*) { return DcOk(); }
FDcResult FDcNoopWriter::WriteWeakObjectReference(const FWeakObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteLazyObjectReference(const FLazyObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteSoftObjectReference(const FSoftObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteSoftClassReference(const FSoftObjectPtr&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInterfaceReference(const FScriptInterface&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteFieldPath(const FFieldPath& Value) { return DcOk(); }
FDcResult FDcNoopWriter::WriteDelegate(const FScriptDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt8(const int8&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt16(const int16&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt32(const int32&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteInt64(const int64&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt8(const uint8&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt16(const uint16&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt32(const uint32&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteUInt64(const uint64&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteFloat(const float&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteDouble(const double&) { return DcOk(); }
FDcResult FDcNoopWriter::WriteBlob(const FDcBlobViewData&) { return DcOk(); }

FName FDcNoopWriter::ClassId() { return FName(TEXT("DcNoopWriter")); }
FName FDcNoopWriter::GetId() { return ClassId(); }

