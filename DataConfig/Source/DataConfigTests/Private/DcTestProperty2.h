#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty2.generated.h"

USTRUCT()
struct FDcTestStructSimple
{
	GENERATED_BODY()
	UPROPERTY() FName NameField;
	UPROPERTY() FString StrField;
};

USTRUCT()
struct FDcTestStructNest1
{
	GENERATED_BODY()
	UPROPERTY() FName NameField;
	UPROPERTY() FDcTestStructSimple StructField;
};

USTRUCT()
struct FDcTestStructNest2
{
	GENERATED_BODY()
	UPROPERTY() TMap<FString, FDcTestStructNest1> StrStructMapField;
};

USTRUCT()
struct FDcTestStructNest3
{
	GENERATED_BODY()
	UPROPERTY() FDcTestStructNest2 StructField;
};

USTRUCT()
struct FDcTestStructNest4
{
	GENERATED_BODY()
	UPROPERTY() TArray<FDcTestStructNest3> StructArrayField;
};






