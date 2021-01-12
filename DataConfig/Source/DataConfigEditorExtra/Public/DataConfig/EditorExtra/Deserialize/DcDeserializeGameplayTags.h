#pragma once

#include "GameplayTags/Public/GameplayTags.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeGameplayTags.generated.h"

/// Deserialize `FGameplayTag` from a hex string

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayTag(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

} // namespace DcEditorExtra

USTRUCT()
struct FDcEditorExtraTestStructWithGameplayTag1
{
	GENERATED_BODY()

	UPROPERTY() FGameplayTag TagField1;
	UPROPERTY() FGameplayTag TagField2;

};
