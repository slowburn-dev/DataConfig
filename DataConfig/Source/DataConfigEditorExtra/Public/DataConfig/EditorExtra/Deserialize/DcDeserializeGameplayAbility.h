#pragma once

///  Populate GameplayAbility / GameplayEffect instance fields from JSON

#include "DataConfig/DcTypes.h"
#include "AssetTypeActions_Base.h"

class UGameplayAbility;
class UGameplayEffect;
struct FDcJsonReader;

namespace DcEditorExtra
{

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcJsonReader& Reader);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcJsonReader& Reader);

class DATACONFIGEDITOREXTRA_API FAssetTypeActions_DcGameplayAbility : public FAssetTypeActions_Base
{
	FText GetName() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;

	void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	bool HasActions(const TArray<UObject*>& InObjects) const override;
	UClass* GetSupportedClass() const override;
};

class DATACONFIGEDITOREXTRA_API FAssetTypeActions_DcGameplayEffect : public FAssetTypeActions_Base
{
	FText GetName() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;

	void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	bool HasActions(const TArray<UObject*>& InObjects) const override;
	UClass* GetSupportedClass() const override;
};

} // namespace DcEditorExtra

