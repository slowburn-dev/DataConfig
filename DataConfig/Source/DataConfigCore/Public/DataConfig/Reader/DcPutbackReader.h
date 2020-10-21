#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Misc/DcDataVariant.h"

struct DATACONFIGCORE_API FDcPutbackReader : public FDcReader
{
	FDcPutbackReader(FDcReader* InReader)
		: Reader(InReader)
	{}

	EDataEntry Peek() override;

	FDcResult ReadNil(FContextStorage* CtxPtr) override;
	FDcResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FDcResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FDcResult ReadClassRoot(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadClassEnd(FDcClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FDcResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FDcResult ReadMapEnd(FContextStorage* CtxPtr) override;
	FDcResult ReadArrayRoot(FContextStorage* CtxPtr) override;
	FDcResult ReadArrayEnd(FContextStorage* CtxPtr) override;
	FDcResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) override;

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

