#pragma once

#include "CoreMinimal.h"
#include "Writer/Writer.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPrettyPrintWriter : public FWriter
{
	FPrettyPrintWriter(FOutputDevice& InOutput);
	~FPrettyPrintWriter();

	FResult Peek(EDataEntry Next) override;
	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;
	FResult WriteStructRoot(const FName& Name) override;
	FResult WriteStructEnd(const FName& Name) override;

	FOutputDevice& Output;
	FString Indent;
};



}	// namespace DataConfig

