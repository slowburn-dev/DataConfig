#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"

namespace DcDeserializeUtils {

extern FString DC_STR_META_TYPE;
extern FString DC_STR_META_PATH;

extern FName DC_NAME_META_TYPE;
extern FName DC_NAME_META_PATH;

const DATACONFIGCORE_API TBasicArray<FName>& GetAllNameMetas();
const DATACONFIGCORE_API TBasicArray<FString>& GetAllStringMetas();

bool DATACONFIGCORE_API IsMeta(const FName& Name);
bool DATACONFIGCORE_API IsMeta(const FString& Str);


}	// namespace DcDeserializeUtils;



