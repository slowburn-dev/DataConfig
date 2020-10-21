#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"

extern FString DC_STR_META_TYPE;
extern FString DC_STR_META_PATH;

extern FName DC_NAME_META_TYPE;
extern FName DC_NAME_META_PATH;

const TBasicArray<FName>& DcGetAllNameMetas();
const TBasicArray<FString>& DcGetAllStringMetas();

bool DcIsMeta(const FName& Name);
bool DcIsMeta(const FString& Str);


