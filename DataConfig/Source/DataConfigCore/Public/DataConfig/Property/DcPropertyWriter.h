#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDcPropertyWriter : public FDcWriter, private FNoncopyable
{
	FDcPropertyWriter();
	FDcPropertyWriter(FDcPropertyDatum Datum);

	FDcResult Peek(EDataEntry Next) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteName(const FName& Value) override;
	FDcResult WriteString(const FString& Value) override;

	FDcResult WriteStructRoot(const FName& Name) override;
	FDcResult WriteStructEnd(const FName& Name) override;

	FDcResult WriteClassRoot(const FDcClassPropertyStat& Class) override;
	FDcResult WriteClassEnd(const FDcClassPropertyStat& Class) override;

	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;

	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;

	FDcResult WriteReference(UObject* Value) override;

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
};



}	// namespace DataConfig




