#pragma once

///  Populate GameplayAbility / GameplayEffect instance fields from JSON

#include "DataConfig/DcTypes.h"
#include "AssetTypeActions_Base.h"

class UGameplayAbility;
class UGameplayEffect;

namespace DcEditorExtra
{

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, const TCHAR* Str);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, const TCHAR* Str);

class DATACONFIGEDITOREXTRA_API FAssetTypeActions_DcGameplayAbility : public FAssetTypeActions_Base
{
	FText GetName() const override;
	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	bool HasActions(const TArray<UObject*>& InObjects) const override;
	UClass* GetSupportedClass() const override;
};

class DATACONFIGEDITOREXTRA_API FAssetTypeActions_DcGameplayEffect : public FAssetTypeActions_Base
{
	FText GetName() const override;
	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	bool HasActions(const TArray<UObject*>& InObjects) const override;
	UClass* GetSupportedClass() const override;
};

} // namespace DcEditorExtra

