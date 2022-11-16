#pragma once

#include "CoreMinimal.h"
#include "DcTestUE5.generated.h"

UCLASS()
class UDcTest5ClassBase : public UObject
{
    GENERATED_BODY()

    UPROPERTY() FString BaseStrField1;
};

UCLASS()
class UDcTest5ClassDerived : public UDcTest5ClassBase
{
    GENERATED_BODY()

    UPROPERTY() FString DerivedStrField1;
};

USTRUCT()
struct FDcTest5ObjectPtrAlpha
{
    GENERATED_BODY()

    UPROPERTY() TObjectPtr<UObject> ObjPtrField1;
    UPROPERTY() TObjectPtr<UObject> ObjPtrField2;
};

USTRUCT()
struct FDcTestLWC
{
	GENERATED_BODY()

    UPROPERTY() FVector VectorField;
    UPROPERTY() FVector3f Vector3fField;
    UPROPERTY() FVector3d Vector3dField;
};

