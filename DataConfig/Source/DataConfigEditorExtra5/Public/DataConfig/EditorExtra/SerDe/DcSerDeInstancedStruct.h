#pragma once

/// Handle `FInstancedStruct` serialization
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

#include "InstancedStruct.h"
#include "Engine/DataAsset.h"

#include "DcSerDeInstancedStruct.generated.h"

namespace DcExtra
{

DATACONFIGEDITOREXTRA5_API FDcResult DcHandlerDeserializeInstancedStruct(
    FDcDeserializeContext& Ctx, 
    TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
);

DATACONFIGEDITOREXTRA5_API FDcResult DcHandlerSerializeInstancedStruct(
    FDcSerializeContext& Ctx,
    TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType
);

DATACONFIGEDITOREXTRA5_API EDcDeserializePredicateResult PredicateIsInstancedStruct(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA5_API FDcResult HandlerInstancedStructDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA5_API EDcSerializePredicateResult PredicateIsInstancedStruct(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA5_API FDcResult HandlerInstancedStructSerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

USTRUCT()
struct FDcEditorExtra5InstancedStruct1
{
    GENERATED_BODY()

    UPROPERTY() FInstancedStruct InstancedStruct1;
    UPROPERTY() FInstancedStruct InstancedStruct2;
    UPROPERTY() FInstancedStruct InstancedStruct3;
};

USTRUCT()
struct FDcEditorExtra5InstancedStruct2
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct ShapeField1;

    UPROPERTY(EditAnywhere, meta = (BaseStruct = "DcStructShapeBase"))
	FInstancedStruct ShapeField2;
};

UCLASS()
class UDcEditorExtra5InstancedStructDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct InstancedStruct1;

    UPROPERTY(EditAnywhere, meta = (BaseStruct = "DcStructShapeBase"))
	FInstancedStruct InstancedStruct2;

    UPROPERTY(EditAnywhere)
	FInstancedStruct InstancedStruct3;
};
