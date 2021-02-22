#pragma once

///  Populate GameplayAbility / GameplayEffect instance fields from JSON

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"

class UGameplayAbility;
class UGameplayEffect;
struct FDcJsonReader;

namespace DcEditorExtra
{

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcJsonReader& Reader);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcJsonReader& Reader);

DATACONFIGEDITOREXTRA_API TSharedRef<FExtender> GameplayAbilityEffectExtender(const TArray<FAssetData>& SelectedAssets);

} // namespace DcEditorExtra

