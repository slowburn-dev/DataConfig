#pragma once

///	Arbitrary struct <-> `FDcInlineStruct`
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Extra/Types/DcInlineStruct.h"
#include "DcSerDeInlineStruct.generated.h"

namespace DcExtra
{

template<typename TInlineStruct>
struct DATACONFIGEXTRA_API TDcInlineStructDeserialize
{
	static FDcResult DcHandlerDeserializeInlineStruct(
		FDcDeserializeContext& Ctx,
		TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
	);
	static FDcResult HandlerDcInlineStructDeserialize(FDcDeserializeContext& Ctx);

	static UScriptStruct* StaticStruct();
};

template<typename TInlineStruct>
struct DATACONFIGEXTRA_API TDcInlineStructSerialize
{
	static FDcResult DcHandlerSerializeInlineStruct(
		FDcSerializeContext& Ctx,
		TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType
	);

	static FDcResult HandlerDcInlineStructSerialize(FDcSerializeContext& Ctx);

	static UScriptStruct* StaticStruct();
};

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestWithInlineStruct1
{
	GENERATED_BODY()

	UPROPERTY() FDcInlineStruct64 InlineField1;
	UPROPERTY() FDcInlineStruct64 InlineField2;
};

USTRUCT()
struct FDcExtraTestStruct128
{
	GENERATED_BODY()

	uint8 Buf[128];
};


