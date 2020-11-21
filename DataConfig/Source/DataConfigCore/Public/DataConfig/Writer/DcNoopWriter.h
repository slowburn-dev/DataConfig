#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcNoopWriter : public FDcWriter
{
	FDcNoopWriter();
	~FDcNoopWriter();

	FDcResult WriteNext(EDcDataEntry) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool) override;
	FDcResult WriteName(const FName&) override;
	FDcResult WriteString(const FString&) override;
	FDcResult WriteText(const FText& Value) override;
	FDcResult WriteEnum(const FDcEnumData& Value) override;

	FDcResult WriteStructRoot(const FName&) override;
	FDcResult WriteStructEnd(const FName&) override;
	FDcResult WriteClassRoot(const FDcClassPropertyStat&) override;
	FDcResult WriteClassEnd(const FDcClassPropertyStat&) override;
	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;
	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;
	FDcResult WriteSetRoot() override;
	FDcResult WriteSetEnd() override;
	FDcResult WriteObjectReference(const UObject*) override;

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

};




