#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcNoopWriter : public FDcWriter
{
	FDcResult PeekWrite(EDcDataEntry, bool* bOutOk) override;

	FDcResult WriteNone() override;
	FDcResult WriteBool(bool) override;
	FDcResult WriteName(const FName&) override;
	FDcResult WriteString(const FString&) override;
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

	FDcResult WriteObjectReference(const UObject*) override;
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

	FDcResult WriteInt8(const int8&) override;
	FDcResult WriteInt16(const int16&) override;
	FDcResult WriteInt32(const int32&) override;
	FDcResult WriteInt64(const int64&) override;

	FDcResult WriteUInt8(const uint8&) override;
	FDcResult WriteUInt16(const uint16&) override;
	FDcResult WriteUInt32(const uint32&) override;
	FDcResult WriteUInt64(const uint64&) override;

	FDcResult WriteFloat(const float&) override;
	FDcResult WriteDouble(const double&) override;
	FDcResult WriteBlob(const FDcBlobViewData& Value) override;

	static FName ClassId(); 
	FName GetId() override;
};




