#pragma once

/// Nested examples
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DcSerDeNested.generated.h"


namespace DcExtra
{

DATACONFIGEXTRA_API FDcResult HandlerVector2DDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGEXTRA_API FDcResult HandlerVector2DSerialize(FDcSerializeContext& Ctx);

#if WITH_EDITORONLY_DATA
DATACONFIGEXTRA_API FDcResult HandlerGrid2DDeserialize(FDcDeserializeContext& Ctx);
DATACONFIGEXTRA_API FDcResult HandlerGrid2DSerialize(FDcSerializeContext& Ctx);
#endif // WITH_EDITORONLY_DATA

} // namespace DcExtra

USTRUCT()
struct FDcGrid2D
{
    GENERATED_BODY()

    UPROPERTY() TArray<int> Data;
};


USTRUCT()
struct FDcExtraTestNested_Vec2
{
    GENERATED_BODY()

    UPROPERTY() TArray<FVector2D> Vec2ArrayField1;
    UPROPERTY() TArray<FVector2D> Vec2ArrayField2;
};


USTRUCT()
struct FDcExtraTestNested_Grid
{
    GENERATED_BODY()

    UPROPERTY(meta=(DcWidth = 2, DcHeight = 2)) FDcGrid2D GridField1;
    UPROPERTY(meta=(DcWidth = 3, DcHeight = 4)) FDcGrid2D GridField2;
};


