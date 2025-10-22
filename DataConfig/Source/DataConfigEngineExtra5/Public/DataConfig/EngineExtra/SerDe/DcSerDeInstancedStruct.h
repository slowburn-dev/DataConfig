#pragma once

/// Handle `FInstancedStruct` serialization
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"

#include "Misc/EngineVersionComparison.h"
#if UE_VERSION_OLDER_THAN(5, 5, 0)
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif // !UE_VERSION_OLDER_THAN(5, 5, 0)

#include "Engine/DataAsset.h"

#include "DcSerDeInstancedStruct.generated.h"

namespace DcExtra
{

DATACONFIGENGINEEXTRA5_API FDcResult DcHandlerDeserializeInstancedStruct(
	FDcDeserializeContext& Ctx,
	TFunctionRef<FDcResult(FDcDeserializeContext&, const FString&, UScriptStruct*&)> FuncLocateStruct
);

DATACONFIGENGINEEXTRA5_API FDcResult DcHandlerSerializeInstancedStruct(
	FDcSerializeContext& Ctx,
	TFunctionRef<FString(UScriptStruct* InStruct)> FuncWriteStructType
);

DATACONFIGENGINEEXTRA5_API FDcResult HandlerInstancedStructDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGENGINEEXTRA5_API FDcResult HandlerInstancedStructSerialize(FDcSerializeContext& Ctx);

} // namespace DcExtra

USTRUCT()
struct FDcEngineExtra5InstancedStruct1
{
	GENERATED_BODY()

	UPROPERTY() FInstancedStruct InstancedStruct1;
	UPROPERTY() FInstancedStruct InstancedStruct2;
	UPROPERTY() FInstancedStruct InstancedStruct3;
};

USTRUCT()
struct FDcEngineExtra5InstancedStruct2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct ShapeField1;

	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct ShapeField2;

};

UCLASS()
class UDcEngineExtra5InstancedStructDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct InstancedStruct1;

	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/DataConfigExtra.DcStructShapeBase"))
	FInstancedStruct InstancedStruct2;

	UPROPERTY(EditAnywhere)
	FInstancedStruct InstancedStruct3;
};
