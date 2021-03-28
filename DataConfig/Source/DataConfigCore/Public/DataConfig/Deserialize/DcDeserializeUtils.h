#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "Containers/BasicArray.h"

struct FDcReader;
struct FDcWriter;

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

DATACONFIGCORE_API FDcResult ExpectMetaKey(const FString& Actual, const TCHAR* Expect);
DATACONFIGCORE_API FDcResult TryStaticFindObject(UClass* Class, UObject* Outer, const TCHAR* Name, bool ExactClass, UObject*& OutObject);
	
template<typename T>
FDcResult TryFindObject(UObject* Outer, const TCHAR* Name, bool ExactClass, T*& OutObject)
{
	return TryStaticFindObject(T::StaticClass(), Outer, Name, ExactClass, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticLoadObject(UClass* Class, UObject* Outer, const TCHAR* LoadPath, UObject*& OutObject);
DATACONFIGCORE_API FDcResult TryLocateObjectByString(UClass* Class, const FString& Str, UObject*& OutObject);
	
DATACONFIGCORE_API FDcResult ExpectLhsChildOfRhs(UClass* Lhs, UClass* Rhs);
DATACONFIGCORE_API FDcResult ExpectNonAbstract(UClass* Class);


}	// namespace DcDeserializeUtils



