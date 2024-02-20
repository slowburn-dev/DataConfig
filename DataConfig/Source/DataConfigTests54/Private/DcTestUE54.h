#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "DcTestUE54.generated.h"

USTRUCT()
struct FDcInnerStruct54
{
    GENERATED_BODY()
    
    UPROPERTY() FString StrField;
    UPROPERTY() int IntField;
};

USTRUCT()
struct FDcTestOptional
{
    GENERATED_BODY()

    UPROPERTY() TOptional<float> OptionalFloatField1;
    UPROPERTY() TOptional<float> OptionalFloatField2;

    UPROPERTY() TOptional<FString> OptionalStrField1;
    UPROPERTY() TOptional<FString> OptionalStrField2;

    UPROPERTY() TOptional<FDcInnerStruct54> OptionalStructField1;
    UPROPERTY() TOptional<FDcInnerStruct54> OptionalStructField2;
};
