#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Property/DcPropertyDatum.h"

#include "DcJsonConverter.generated.h"

namespace DcExtra
{

/// Stock `FJsonObjectConverter` method re-implementations

static bool JsonObjectReaderToUStruct(FDcReader* Reader, FDcPropertyDatum Datum);

template<typename OutStructType>
static bool JsonObjectStringToUStruct(const FString& JsonString, OutStructType* OutStruct)
{
	FDcJsonReader Reader(JsonString);
	FDcPropertyDatum Datum(TBaseStructure<OutStructType>::Get(), OutStruct);
	return JsonObjectReaderToUStruct(&Reader, Datum);
}

static bool UStructToJsonObjectString(FDcPropertyDatum Datum, FDcWriter* Writer);


template<typename InStructType>
static bool UStructToJsonObjectString(const InStructType& InStruct, FString& OutJsonString)
{
	static FDcJsonWriter::ConfigType _JSON_CONVERTER_CONFIG = []
	{
		FDcJsonWriter::ConfigType Config = FDcJsonWriter::DefaultConfig;
		Config.IndentLiteral = TEXT("\t");
		Config.LineEndLiteral = LINE_TERMINATOR;
		Config.LeftSpacingLiteral = TEXT("");
		Config.bNestedArrayStartsOnNewLine = false;
		Config.bNestedObjectStartsOnNewLine = true;
		return Config;
	}();

	FDcJsonWriter Writer(_JSON_CONVERTER_CONFIG);
	FDcPropertyDatum Datum(TBaseStructure<InStructType>::Get(), (InStructType*)&InStruct);
	bool bRet = UStructToJsonObjectString(Datum, &Writer);
	if (bRet)
	{
		OutJsonString = Writer.Sb.ToString();
	}
	return bRet;
}

	
} // namespace DcExtra

USTRUCT()
struct FDcTestJsonConverterInner1
{
	GENERATED_BODY()

	UPROPERTY() TArray<FString> StrArrayField;
	UPROPERTY() TMap<FString, int> StrIntMapField;
};

USTRUCT()
struct FDcTestJsonConverter1
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
	UPROPERTY() FDcTestJsonConverterInner1 NestField;
	UPROPERTY() int IntField;
	UPROPERTY() bool BoolField;
};

USTRUCT()
struct FMovie
{
	GENERATED_BODY()

	UPROPERTY() FString Name;
	UPROPERTY() FString ReleaseDate;
	UPROPERTY() TArray<FString> Genres;
};

USTRUCT()
struct FDcTestJsonConverterArrayDim1
{
	GENERATED_BODY()
	
	UPROPERTY() FString StrArr[2];
	UPROPERTY() int IntArr[3];
	UPROPERTY() FMovie MovieArr[2];
};


