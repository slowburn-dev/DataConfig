#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DataConfig
{

//	TODO move statics to StartUp
FString STR_META_TYPE = FString(TEXT("$type"));
FString STR_META_PATH = FString(TEXT("$path"));

FName NAME_META_TYPE = FName(TEXT("$type"));
FName NAME_META_PATH = FName(TEXT("$path"));

const TBasicArray<FName>& GetAllNameMetas()
{
	static TBasicArray<FName> ALL_NAME_METAS;
	if (ALL_NAME_METAS.Num() == 0)
	{
		ALL_NAME_METAS.Emplace(NAME_META_TYPE);
		ALL_NAME_METAS.Emplace(NAME_META_PATH);
	}
	return ALL_NAME_METAS;
}


const TBasicArray<FString>& GetAllStringMetas()
{
	static TBasicArray<FString> ALL_STR_METAS;
	if (ALL_STR_METAS.Num() == 0)
	{
		ALL_STR_METAS.Emplace(STR_META_TYPE);
		ALL_STR_METAS.Emplace(STR_META_PATH);
	}
	return ALL_STR_METAS;
}

bool IsMeta(const FName& Name)
{
	for (const FName& Cur : GetAllNameMetas())
	{
		if (Name == Cur)
			return true;
	}

	return false;
}

bool IsMeta(const FString& Str)
{
	for (const FString& Cur : GetAllStringMetas())
	{
		if (Str == Cur)
			return true;
	}

	return false;
}

}; // namespace DataConfig


