#pragma once

///	Deserialize arbitrary struct into a `FDcAnyStruct`
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Extra/Types/DcAnyStruct.h"
#include "DcDeserializeAnyStruct.generated.h"

namespace DcExtra 
{

DATACONFIGEXTRA_API EDcDeserializePredicateResult PredicateIsDcAnyStruct(FDcDeserializeContext& Ctx);

DATACONFIGEXTRA_API FDcResult HandlerDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

}	//	namespace DcExtra

USTRUCT()
struct FDcExtraTestSimpleStruct1 
{
	GENERATED_BODY()

	UPROPERTY() FName NameField;
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
};


