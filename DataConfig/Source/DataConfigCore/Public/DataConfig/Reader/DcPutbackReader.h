#pragma once

#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Misc/DcDataVariant.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPutbackReader : public FReader
{
	FPutbackReader(FReader* InReader)
		: Reader(InReader)
	{}

	EDataEntry Peek() override;

	FResult ReadNil(FContextStorage* CtxPtr) override;
	FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadStructRoot(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadStructEnd(FName* OutNamePtr, FContextStorage* CtxPtr) override;
	FResult ReadClassRoot(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FResult ReadClassEnd(FClassPropertyStat* OutClassPtr, FContextStorage* CtxPtr) override;
	FResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FResult ReadMapEnd(FContextStorage* CtxPtr) override;
	FResult ReadArrayRoot(FContextStorage* CtxPtr) override;
	FResult ReadArrayEnd(FContextStorage* CtxPtr) override;
	FResult ReadReference(UObject** OutPtr, FContextStorage* CtxPtr) override;

	template<typename T>
	void Putback(T&& InValue);

	TArray<FDataVariant> Cached;
	FReader* Reader;
};

template<typename T>
void FPutbackReader::Putback(T&& InValue)
{
	Cached.Insert(Forward<T>(InValue), 0);
}

}	// namespace DataConfig

