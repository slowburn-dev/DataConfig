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

USTRUCT()
struct FMapContainer1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FString, FString> StrStrMap;
	UPROPERTY(EditAnywhere) TMap<FName, bool> NameBoolMap;

};

USTRUCT()
struct FMapOfStruct1
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FString, FTestStruct_Alpha> StrStructMap;
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


