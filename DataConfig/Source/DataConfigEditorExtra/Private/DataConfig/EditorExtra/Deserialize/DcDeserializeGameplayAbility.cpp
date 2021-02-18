#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"

namespace DcEditorExtra
{

FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, const TCHAR* Str)
{
	return DcOk();
}

FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, const TCHAR* Str)
{
	return DcOk();
}

FText FAssetTypeActions_DcGameplayAbility::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayAbility", "DataConfig GameplayAbility Action"); 
}

void FAssetTypeActions_DcGameplayAbility::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

bool FAssetTypeActions_DcGameplayAbility::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayAbility::GetSupportedClass() const
{
	return nullptr;
}

FText FAssetTypeActions_DcGameplayEffect::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayEffect", "DataConfig GameplayEffect Action"); 
}

void FAssetTypeActions_DcGameplayEffect::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

bool FAssetTypeActions_DcGameplayEffect::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayEffect::GetSupportedClass() const
{
	return nullptr;
}

} // namespace DcEditorExtra
