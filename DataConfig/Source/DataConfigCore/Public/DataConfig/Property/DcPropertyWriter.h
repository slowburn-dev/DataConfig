#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcPropertyWriter : public FDcWriter, private FNoncopyable
{
	FDcPropertyWriter();
	FDcPropertyWriter(FDcPropertyDatum Datum);

	FDcResult WriteNext(EDcDataEntry Next) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteString(const FString& Value) override;
	FDcResult WriteEnum(const FDcEnumData& Value) override;

	FDcResult WriteStructRoot(const FName& Name) override;
	FDcResult WriteStructEnd(const FName& Name) override;

	FDcResult WriteClassRoot(const FDcClassPropertyStat& Class) override;
	FDcResult WriteClassEnd(const FDcClassPropertyStat& Class) override;

	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;

	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;

	FDcResult WriteSetRoot() override;
	FDcResult WriteSetEnd() override;

	FDcResult WriteReference(UObject* Value) override;

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

	//	skip current write at current position
	FDcResult SkipWrite();
	//	get the next write property
	FDcResult PeekWriteProperty(UField** OutProperty);
	//	manual writing
	FDcResult WriteDataEntry(UClass* ExpectedPropertyClass, FDcPropertyDatum& OutDatum);

	//	manual writing supporting
	void PushTopClassPropertyState(FDcPropertyDatum& Datum);

	struct FPropertyState
	{
		using ImplStorageType = TDcAlignedStorage<64>::Type;
		ImplStorageType ImplStorage;
	};

	TArray<FPropertyState, TInlineAllocator<4>> States;

	FDcDiagnosticHighlight FormatHighlight();

};



