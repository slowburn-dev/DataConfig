#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Misc/DcDataVariant.h"

struct DATACONFIGCORE_API FDcPutbackReader : public FDcReader
{
	FDcPutbackReader(FDcReader* InReader)
		: Reader(InReader)
	{}

	EDcDataEntry Peek() override;

	FDcResult ReadNil() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadStructRoot(FName* OutNamePtr) override;
	FDcResult ReadStructEnd(FName* OutNamePtr) override;
	FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr) override;
	FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;
	FDcResult ReadReference(UObject** OutPtr) override;

	template<typename T>
	void Putback(T&& InValue);

	TArray<FDcDataVariant> Cached;
	FDcReader* Reader;
};

template<typename T>
void FDcPutbackReader::Putback(T&& InValue)
{
	Cached.Insert(Forward<T>(InValue), 0);
}

