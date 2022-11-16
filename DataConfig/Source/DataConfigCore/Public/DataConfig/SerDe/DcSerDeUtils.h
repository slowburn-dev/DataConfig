#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

struct FDcReader;
struct FDcWriter;
struct FDcSerializeContext;
struct FDcDeserializeContext;

namespace DcSerDeUtils
{

DATACONFIGCORE_API bool IsMeta(const FString& Str);
DATACONFIGCORE_API FDcResult ExpectMetaKey(const FString& Actual, const TCHAR* Expect);

DATACONFIGCORE_API FDcResult DispatchPipeVisit(EDcDataEntry Next, FDcReader* Reader, FDcWriter* Writer);

/// FindFirst
/// - calls `FindObject(ANY_PACKAGE)` pre UE5.1
/// - calls `FindFirstObject()` post UE5.1
/// - fails when object not loaded yet
/// Find
/// - calls `StaticFindObject`
/// - fails when object not loaded yet
/// Load
/// - calls `StaticLoadObject`
/// - internally do a find first, loads if object isn't loaded
/// - equal to `Find` when it's already loaded
/// Locate
///	- DataConfig specific heuristic
/// - `Load` when str is "/Foo/Bar" like, otherwise `FindFirst`.

DATACONFIGCORE_API UObject* StaticFindFirstObject(UClass* Class, const TCHAR* Name, bool bExactClass);

template<typename T>
T* FindFirstObject(const TCHAR* Name, bool bExactClass)
{
	return (T*)DcSerDeUtils::StaticFindFirstObject(T::StaticClass(), Name, bExactClass);
}

DATACONFIGCORE_API FDcResult TryStaticFindFirstObject(UClass* Class, const TCHAR* Name, bool bExactClass, UObject*& OutObject);

template<typename T>
FDcResult TryFindFirstObject(const TCHAR* Name, bool bExactClass, T*& OutObject)
{
	return TryStaticFindFirstObject(T::StaticClass(), Name, bExactClass, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticFindObject(UClass* Class, const FName& PackageName , const FName& AssetName, bool bExactClass, UObject*& OutObject);
DATACONFIGCORE_API FDcResult TryStaticFindObject(UClass* Class, UObject* Outer, const TCHAR* Name, bool bExactClass, UObject*& OutObject);

template<typename T>
FDcResult TryFindObject(UObject* Outer, const TCHAR* Name, bool bExactClass, T*& OutObject)
{
	return TryStaticFindObject(T::StaticClass(), Outer, Name, bExactClass, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticLoadObject(UClass* Class, UObject* Outer, const TCHAR* LoadPath, UObject*& OutObject);

template<typename T>
FDcResult TryLoadObject(UObject* Outer, const TCHAR* LoadPath, T*& OutObject)
{
	return TryStaticLoadObject(T::StaticClass(), Outer, LoadPath, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticLocateObject(UClass* Class, FStringView Str, UObject*& OutObject);

template<typename T>
FDcResult TryLocateObject(FStringView Str, T*& OutObject)
{
	return TryStaticLocateObject(T::StaticClass(), Str, (UObject*&)OutObject);
}
	
DATACONFIGCORE_API FDcResult ExpectLhsChildOfRhs(UClass* Lhs, UClass* Rhs);
DATACONFIGCORE_API FDcResult ExpectNonAbstract(UClass* Class);

DATACONFIGCORE_API FString FormatObjectName(UObject* Object); 

DATACONFIGCORE_API FDcResult DispatchNoopRead(EDcDataEntry Next, FDcReader* Reader);
DATACONFIGCORE_API FDcResult ReadNoopConsumeValue(FDcReader* Reader);

} // namespace DcSerDeUtils



