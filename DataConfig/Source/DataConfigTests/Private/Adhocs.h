#pragma once

#include "Reader/Reader.h"
#include "Writer/Writer.h"
#include "Adhocs.generated.h"

USTRUCT()
struct FTestStruct_Alpha
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) bool ABool;
	UPROPERTY(EditAnywhere) FString AStr;
};

USTRUCT()
struct FNestStruct1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct;
	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct2;
};

UCLASS()
class UTestClass_Alpha : public UObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName AName;
	UPROPERTY(EditAnywhere) bool ABool;
	UPROPERTY(EditAnywhere) FString AStr;

	UPROPERTY(EditAnywhere) FTestStruct_Alpha AStruct;
};


using namespace DataConfig;
struct FPipeVisitor
{
	FReader* Reader;
	FWriter* Writer;

	FPipeVisitor(FReader* InReader, FWriter* InWriter);

	FResult PipeVisit();
};


void PropertyVisitorRoundtrip();


