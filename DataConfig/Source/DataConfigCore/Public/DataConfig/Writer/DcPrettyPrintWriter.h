#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDcPrettyPrintWriter : public FDcWriter
{
	FDcPrettyPrintWriter(FOutputDevice& InOutput);
	~FDcPrettyPrintWriter();

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

	FOutputDevice& Output;
	FString Indent;



};



}	// namespace DataConfig

