#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "Containers/BasicArray.h"

namespace DcDeserializeUtils {

extern FString DC_STR_META_TYPE;
extern FString DC_STR_META_PATH;

extern FName DC_NAME_META_TYPE;
extern FName DC_NAME_META_PATH;

DATACONFIGCORE_API const TBasicArray<FName>& GetAllNameMetas();
DATACONFIGCORE_API const TBasicArray<FString>& GetAllStringMetas();

DATACONFIGCORE_API bool IsMeta(const FName& Name);
DATACONFIGCORE_API bool IsMeta(const FString& Str);

DATACONFIGCORE_API FDcResult DispatchPipeVisit(EDcDataEntry Next, FDcReader* Reader, FDcWriter* Writer);

}	// namespace DcDeserializeUtils



