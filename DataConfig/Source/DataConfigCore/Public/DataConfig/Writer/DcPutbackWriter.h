#pragma once

#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcPutbackWriter : public FDcWriter
{
	FDcPutbackWriter(FDcWriter* InWriter)
		: Writer(InWriter)
	{}

	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;

	FDcResult WriteNone() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteString(const FString& Value) override;
	FDcResult WriteText(const FText& Value) override;
	FDcResult WriteEnum(const FDcEnumData& Value) override;

	FDcResult WriteStructRootAccess(FDcStructAccess& Access) override;
	FDcResult WriteStructEndAccess(FDcStructAccess& Access) override;
	FDcResult WriteClassRootAccess(FDcClassAccess& Access) override;
	FDcResult WriteClassEndAccess(FDcClassAccess& Access) override;
	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;
	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;
	FDcResult WriteSetRoot() override;
	FDcResult WriteSetEnd() override;
	FDcResult WriteOptionalRoot() override;
	FDcResult WriteOptionalEnd() override;

	FDcResult WriteObjectReference(const UObject* Value) override;
	FDcResult WriteClassReference(const UClass* Value) override;

	FDcResult WriteWeakObjectReference(const FWeakObjectPtr& Value) override;
	FDcResult WriteLazyObjectReference(const FLazyObjectPtr& Value) override;
	FDcResult WriteSoftObjectReference(const FSoftObjectPtr& Value) override;
	FDcResult WriteSoftClassReference(const FSoftObjectPtr& Value) override;
	FDcResult WriteInterfaceReference(const FScriptInterface& Value) override;

	FDcResult WriteFieldPath(const FFieldPath& Value) override;
	FDcResult WriteDelegate(const FScriptDelegate& Value) override;
	FDcResult WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value) override;
	FDcResult WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value) override;

	FDcResult WriteInt8(const int8& Value) override;
	FDcResult WriteInt16(const int16& Value) override;
	FDcResult WriteInt32(const int32& Value) override;
	FDcResult WriteInt64(const int64& Value) override;

	FDcResult WriteUInt8(const uint8& Value) override;
	FDcResult WriteUInt16(const uint16& Value) override;
	FDcResult WriteUInt32(const uint32& Value) override;
	FDcResult WriteUInt64(const uint64& Value) override;

	FDcResult WriteFloat(const float& Value) override;
	FDcResult WriteDouble(const double& Value) override;

	FDcResult WriteBlob(const FDcBlobViewData& Value) override;

	void FormatDiagnostic(FDcDiagnostic& Diag) override;
	void Putback(EDcDataEntry Entry) { Cached.Insert(Entry, 0); }

	TArray<EDcDataEntry> Cached;
	FDcWriter* Writer;

	static FName ClassId();
	FName GetId() override;

};




