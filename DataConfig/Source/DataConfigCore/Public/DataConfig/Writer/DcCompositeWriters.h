#pragma once

#include "DataConfig/Writer/DcWriter.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FWeakCompositeWriter : public FWriter
{
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

	TArray<FWriter*, TInlineAllocator<4>> Writers;

};


}	// namespace DataConfig


