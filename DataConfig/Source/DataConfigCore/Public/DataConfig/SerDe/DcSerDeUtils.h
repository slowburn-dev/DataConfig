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

/// Find - Strictly Load
/// Load - Strictly Find
/// Locate - Load when str is "/Foo/Bar" like, otherwise Find.

DATACONFIGCORE_API FDcResult TryStaticFindObject(UClass* Class, UObject* Outer, const TCHAR* Name, bool ExactClass, UObject*& OutObject);

template<typename T>
FDcResult TryFindObject(UObject* Outer, const TCHAR* Name, bool ExactClass, T*& OutObject)
{
	return TryStaticFindObject(T::StaticClass(), Outer, Name, ExactClass, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticLoadObject(UClass* Class, UObject* Outer, const TCHAR* LoadPath, UObject*& OutObject);

template<typename T>
FDcResult TryLoadObject(UObject* Outer, const TCHAR* LoadPath, T*& OutObject)
{
	return TryStaticLoadObject(T::StaticClass(), Outer, LoadPath, (UObject*&)OutObject);
}

DATACONFIGCORE_API FDcResult TryStaticLocateObject(UClass* Class, const FString& Str, UObject*& OutObject);

template<typename T>
FDcResult TryLocateObject(const FString& Str, T*& OutObject)
{
	return TryStaticLocateObject(T::StaticClass(), Str, (UObject*&)OutObject);
}
	
DATACONFIGCORE_API FDcResult ExpectLhsChildOfRhs(UClass* Lhs, UClass* Rhs);
DATACONFIGCORE_API FDcResult ExpectNonAbstract(UClass* Class);

DATACONFIGCORE_API FString FormatObjectName(UObject* Object); 

DATACONFIGCORE_API FDcResult DispatchNoopRead(EDcDataEntry Next, FDcReader* Reader);
DATACONFIGCORE_API FDcResult ReadNoopConsumeValue(FDcReader* Reader);

} // namespace DcSerDeUtils



