#include "DataConfig/Deserialize/DcDeserializeUtils.h"

namespace DcDeserializeUtils {

//	TODO move statics to StartUp
FString DC_STR_META_TYPE = FString(TEXT("$type"));
FString DC_STR_META_PATH = FString(TEXT("$path"));

FName DC_NAME_META_TYPE = FName(TEXT("$type"));
FName DC_NAME_META_PATH = FName(TEXT("$path"));

const TBasicArray<FName>& DcGetAllNameMetas()
{
	static TBasicArray<FName> ALL_NAME_METAS;
	if (ALL_NAME_METAS.Num() == 0)
	{
		ALL_NAME_METAS.Emplace(DC_NAME_META_TYPE);
		ALL_NAME_METAS.Emplace(DC_NAME_META_PATH);
	}
	return ALL_NAME_METAS;
}


const TBasicArray<FString>& GetAllStringMetas()
{
	static TBasicArray<FString> ALL_STR_METAS;
	if (ALL_STR_METAS.Num() == 0)
	{
		ALL_STR_METAS.Emplace(DC_STR_META_TYPE);
		ALL_STR_METAS.Emplace(DC_STR_META_PATH);
	}
	return ALL_STR_METAS;
}

bool IsMeta(const FName& Name)
{
	for (const FName& Cur : DcGetAllNameMetas())
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

}	// namespace DcDeserializeUtils

