#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"

namespace DcDeserializeUtils {

extern FString DC_STR_META_TYPE;
extern FString DC_STR_META_PATH;

extern FName DC_NAME_META_TYPE;
extern FName DC_NAME_META_PATH;

const TBasicArray<FName>& GetAllNameMetas();
const TBasicArray<FString>& GetAllStringMetas();

bool IsMeta(const FName& Name);
bool IsMeta(const FString& Str);

bool CastScriptStruct(const FDcPropertyDatum& Datum);

}	// namespace DcDeserializeUtils;



