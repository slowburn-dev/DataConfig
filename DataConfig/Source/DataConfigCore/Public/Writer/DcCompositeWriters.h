#pragma once

#include "Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FWeakCompositeWriter : public FWriter
{
	FResult Peek(EDataEntry Next) override;
	FResult WriteBool(bool Value) override;
	FResult WriteName(const FName& Value) override;
	FResult WriteString(const FString& Value) override;
	FResult WriteStructRoot(const FName& Name) override;
	FResult WriteStructEnd(const FName& Name) override;
	FResult WriteClassRoot(const FName& Name) override;
	FResult WriteClassEnd(const FName& Name) override;
	FResult WriteMapRoot() override;
	FResult WriteMapEnd() override;
	FResult WriteArrayRoot() override;
	FResult WriteArrayEnd() override;


	TArray<FWriter*, TInlineAllocator<4>> Writers;
};


}	// namespace DataConfig



