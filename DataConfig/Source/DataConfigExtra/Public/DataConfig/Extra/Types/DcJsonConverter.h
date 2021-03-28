#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
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
	FDcPropertyDatum Datum(OutStructType::StaticStruct(), OutStruct);
	return JsonObjectReaderToUStruct(&Reader, Datum);
}
	
} // namespace DcExtra

USTRUCT()
struct FDcTestJsonConverter1
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
	UPROPERTY() int IntField;
	UPROPERTY() bool BoolField;
};

