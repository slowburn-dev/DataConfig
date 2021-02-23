#pragma once

///  Populate GameplayAbility / GameplayEffect instance fields from JSON

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

class UGameplayAbility;
class UGameplayEffect;
struct FDcReader;

namespace DcEditorExtra
{

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcReader& Reader);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcReader& Reader);

DATACONFIGEDITOREXTRA_API TSharedRef<FExtender> GameplayAbilityEffectExtender(const TArray<FAssetData>& SelectedAssets);

} // namespace DcEditorExtra

