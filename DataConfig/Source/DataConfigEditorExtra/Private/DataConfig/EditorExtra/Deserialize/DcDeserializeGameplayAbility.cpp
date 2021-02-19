#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"

#include "Abilities/GameplayAbility.h"
#include "GameplayAbilityBlueprint.h"
#include "Textures/SlateIcon.h"
#include "ToolMenuSection.h"

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
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayAbility", "DataConfig GameplayAbility"); 
}

FColor FAssetTypeActions_DcGameplayAbility::GetTypeColor() const
{
	return FColor(201, 29, 85); 
}

uint32 FAssetTypeActions_DcGameplayAbility::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FAssetTypeActions_DcGameplayAbility::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UGameplayAbilityBlueprint>> Abilities = GetTypedWeakObjectPtrs<UGameplayAbilityBlueprint>(InObjects);

	if (Abilities.Num() != 1)
		return;

	Section.AddMenuEntry(
		TEXT("DcEditorExtra_LoadFromJson"),
		NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJson", "Load From JSON"), 
		NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadFromJsonTooltip", "Load default values from a JSON file"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([]
			{
				// pass
			}),
			FCanExecuteAction()
		)
	);
}

bool FAssetTypeActions_DcGameplayAbility::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayAbility::GetSupportedClass() const
{
	return UGameplayAbilityBlueprint::StaticClass();
}

FText FAssetTypeActions_DcGameplayEffect::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DcGameplayEffect", "DataConfig GameplayEffect"); 
}

FColor FAssetTypeActions_DcGameplayEffect::GetTypeColor() const
{
	return FColor(201, 29, 85); 
}

uint32 FAssetTypeActions_DcGameplayEffect::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FAssetTypeActions_DcGameplayEffect::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
}

bool FAssetTypeActions_DcGameplayEffect::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

UClass* FAssetTypeActions_DcGameplayEffect::GetSupportedClass() const
{
	return UGameplayEffect::StaticClass();
}

} // namespace DcEditorExtra
