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
	TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> Func
);

DATACONFIGEXTRA_API FDcResult DcHandlerSerializeAnyStruct(
	FDcSerializeContext& Ctx,
	TFunctionRef<FString(UScriptStruct* InStruct)> Func
);

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API EDcSerializePredicateResult PredicateIsDcAnyStruct(FDcSerializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructSerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

USTRUCT()
struct FDcExtraTestSimpleStruct1
{
	GENERATED_BODY()

	UPROPERTY() FName NameField;
	UPROPERTY(meta = (DcSkip)) int IntFieldWithDefault = 253;
};

USTRUCT()
struct FDcExtraTestSimpleStruct2
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
};


DECLARE_DELEGATE(FDcExtraTestSingularAction);

USTRUCT()
struct FDcExtraTestDestructDelegateContainer
{
	GENERATED_BODY()

	FDcExtraTestSingularAction DestructAction;

	~FDcExtraTestDestructDelegateContainer()
	{
		DestructAction.ExecuteIfBound();
	}
};

USTRUCT()
struct FDcExtraTestWithAnyStruct1
{
	GENERATED_BODY()

	UPROPERTY() FDcAnyStruct AnyStructField1;
	UPROPERTY() FDcAnyStruct AnyStructField2;
	UPROPERTY() FDcAnyStruct AnyStructField3;
};


