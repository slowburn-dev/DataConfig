#pragma once

#include "CoreMinimal.h"
#include "DcTestBlurb.generated.h"

UENUM()
enum class EDcTestExampleEnum
{
	Foo, Bar, Baz
};

USTRUCT()
struct FDcTestExampleStruct
{
	GENERATED_BODY()
	UPROPERTY() FString StrField;
	UPROPERTY() EDcTestExampleEnum EnumField;
	UPROPERTY() TArray<FColor> Colors;
};

USTRUCT()
struct FDcTestExampleSimple
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
	UPROPERTY() int IntField;
};


