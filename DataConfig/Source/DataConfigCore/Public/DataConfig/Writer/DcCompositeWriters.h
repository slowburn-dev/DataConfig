#pragma once

#include "DataConfig/Writer/DcWriter.h"

struct DATACONFIGCORE_API FDcWeakCompositeWriter : public FDcWriter
{
	FDcResult WriteNext(EDcDataEntry Next) override;

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

	TArray<FDcWriter*, TInlineAllocator<4>> Writers;

};


