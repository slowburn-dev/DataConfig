#pragma once

#include "CoreMinimal.h"
#include "Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPrettyPrintWriter : public FWriter
{
	FPrettyPrintWriter(FOutputDevice& InOutput);
	~FPrettyPrintWriter();

	FResult Peek(EDataEntry Next) override;

	FResult WriteNil() override;
	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;
	FResult WriteStructRoot(const FName& Name) override;
	FResult WriteStructEnd(const FName& Name) override;
	FResult WriteClassRoot(const FClassPropertyStat& Class) override;
	FResult WriteClassEnd(const FClassPropertyStat& Class) override;
	FResult WriteMapRoot() override;
	FResult WriteMapEnd() override;
	FResult WriteArrayRoot() override;
	FResult WriteArrayEnd() override;
	FResult WriteReference(UObject* Value) override;

	FOutputDevice& Output;
	FString Indent;



};



}	// namespace DataConfig

