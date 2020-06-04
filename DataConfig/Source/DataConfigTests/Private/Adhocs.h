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

UCLASS()
class UTestObj_Alpha : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere) FString AStr;
	UPROPERTY(VisibleAnywhere) FTestStruct_Alpha AStruct;
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

USTRUCT()
struct FKeyableStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Name;
	UPROPERTY(EditAnywhere) bool Index;

	//	inline friend declaration
	//	https://devdocs.io/cpp/language/friend see #2
	FORCEINLINE friend uint32 GetTypeHash(const FKeyableStruct& In)
	{
		return HashCombine(GetTypeHash(In.Name), GetTypeHash(In.Index));
	}

	//	even concept checked this, still need these 2 operators to be put into struct
	FORCEINLINE friend bool operator==(const FKeyableStruct& Lhs, const FKeyableStruct& Rhs)
	{
		return Lhs.Name == Rhs.Name
			&& Lhs.Index == Rhs.Index;
	}

	FORCEINLINE friend bool operator!=(const FKeyableStruct& Lhs, const FKeyableStruct& Rhs)
	{
		return Lhs != Rhs;
	}


};

USTRUCT()
struct FMapOfStruct2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TMap<FKeyableStruct, FTestStruct_Alpha> StructStructMap;
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


