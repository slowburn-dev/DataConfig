#pragma once

///	Arbitrary struct <-> `FDcAnyStruct`
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DcSerDeAnyStruct.generated.h"

namespace DcExtra
{

DATACONFIGEXTRA_API FDcResult DcHandlerDeserializeAnyStruct(
	FDcDeserializeContext& Ctx, 
	TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
);

DATACONFIGEXTRA_API FDcResult DcHandlerSerializeAnyStruct(
	FDcSerializeContext& Ctx,
	TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType
);

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API EDcSerializePredicateResult PredicateIsDcAnyStruct(FDcSerializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructSerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestWithAnyStruct1
{
	GENERATED_BODY()

	UPROPERTY() FDcAnyStruct AnyStructField1;
	UPROPERTY() FDcAnyStruct AnyStructField2;
	UPROPERTY() FDcAnyStruct AnyStructField3;
};


