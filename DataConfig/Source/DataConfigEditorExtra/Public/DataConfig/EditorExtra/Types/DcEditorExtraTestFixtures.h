#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"
#include "DcEditorExtraTestFixtures.generated.h"

UCLASS()
class UDcEditorExtraInstancedObjectDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Instanced)
    UDcBaseShape* ShapeField1;

    UPROPERTY(EditAnywhere, Instanced)
    UDcBaseShape* ShapeField2;
};


