#pragma once

#include "GameplayTags/Public/GameplayTags.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DcSerDeGameplayTags.generated.h"

/// GameplayTag and GameplayTagContainer SerDe

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayTag(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayTagContainer(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagContainerDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API EDcSerializePredicateResult PredicateIsGameplayTag(FDcSerializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API EDcSerializePredicateResult PredicateIsGameplayTagContainer(FDcSerializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayTagContainerSerialize(FDcSerializeContext& Ctx);


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
