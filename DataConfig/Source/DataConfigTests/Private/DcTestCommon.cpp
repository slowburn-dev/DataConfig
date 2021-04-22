#include "DcTestCommon.h"

#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "Interfaces/IPluginManager.h"

static_assert(DcTypeUtils::TIsUClass<UClass>::Value, "test TIsUClass");
static_assert(!DcTypeUtils::TIsUClass<FDcReader>::Value, "test TIsUClass");

static_assert(!DcTypeUtils::TIsUStruct<UClass>::Value, "test TIsUStruct");
static_assert(!DcTypeUtils::TIsUStruct<FDcReader>::Value, "test TIsUStruct");

static_assert(TIsSame<DcPropertyUtils::TPropertyTypeMap<int32>::Type, FIntProperty>::Value, "test DcPropertyUtils");

static_assert(DcPropertyUtils::TIsInPropertyMap<int32>::Value, "test TIsInPropertyMap");
static_assert(!DcPropertyUtils::TIsInPropertyMap<FDcReader>::Value, "test TIsInPropertyMap");

FString DcGetFixturePath(const FString& Str)
{
#if IS_PROGRAM
	FString Root = FPaths::LaunchDir();
#else
	FString Root = IPluginManager::Get().FindPlugin("DataConfig")->GetBaseDir();
#endif
	FPaths::NormalizeDirectoryName(Root);
	return FString::Printf(TEXT("%s/Tests/%s"), *Root, *Str);
}
