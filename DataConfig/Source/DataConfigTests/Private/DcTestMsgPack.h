#pragma once

#include "CoreMinimal.h"
#include "DcTestMsgPack.generated.h"

USTRUCT()
struct FDcTestMsgPackBlob
{
	GENERATED_BODY()

	UPROPERTY() TArray<uint8> Blob;
};

UENUM()
enum class EDcTestMsgPackCompareType
{
	Skip,
	PrettyPrintEqualable,
	NumberBigNumber,
	Extension,
};

USTRUCT()
struct FMsgPackTestFixture
{
	GENERATED_BODY()

	UPROPERTY() EDcTestMsgPackCompareType Type;
	UPROPERTY() TArray<uint8> Expect;
	UPROPERTY() TArray<FDcTestMsgPackBlob> Actuals;

	UPROPERTY() FString ExpectDoubleStr;

};

USTRUCT()
struct FMsgPackTestGroup
{
	GENERATED_BODY()

	UPROPERTY() FString Name;
	UPROPERTY() TArray<FMsgPackTestFixture> Fixtures;
};

USTRUCT()
struct FMsgPackTestSuite
{
	GENERATED_BODY()
	
	UPROPERTY() TArray<FMsgPackTestGroup> Groups;
};


