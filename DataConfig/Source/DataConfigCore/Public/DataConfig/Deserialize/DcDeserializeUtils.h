#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"

namespace DataConfig
{

extern FString STR_META_TYPE;
extern FString STR_META_PATH;

extern FName NAME_META_TYPE;
extern FName NAME_META_PATH;

const TBasicArray<FName>& GetAllNameMetas();
const TBasicArray<FString>& GetAllStringMetas();

bool IsMeta(const FName& Name);
bool IsMeta(const FString& Str);

}; // namespace DataConfig




