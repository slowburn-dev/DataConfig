#pragma once

#include "GameplayTags/Public/GameplayTags.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeGameplayTags.generated.h"

/// Deserialize `FGameplayTag` from a hex string

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayTag(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayTagContainer(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagContainerDeserialize(FDcDeserializeContext& Ctx);

} // namespace DcEditorExtra

USTRUCT()
struct FDcEditorExtraTestStructWithGameplayTag1
{
	GENERATED_BODY()

	UPROPERTY() FGameplayTag TagField1;
	UPROPERTY() FGameplayTag TagField2;

};

USTRUCT()
struct FDcEditorExtraTestStructWithGameplayTag2
{
	GENERATED_BODY()

	UPROPERTY() FGameplayTagContainer TagContainerField1;
	UPROPERTY() FGameplayTagContainer TagContainerField2;
};
