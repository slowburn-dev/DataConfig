#pragma once

///  Populate GameplayAbility / GameplayEffect instance fields from JSON

#include "CoreMinimal.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

#include "DcDeserializeGameplayAbility.generated.h"

class UGameplayAbility;
class UGameplayEffect;
struct FDcReader;

namespace DcEditorExtra
{

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsGameplayAttribute(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerGameplayAttributeDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcReader& Reader);

DATACONFIGEDITOREXTRA_API FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcReader& Reader);

DATACONFIGEDITOREXTRA_API TSharedRef<FExtender> GameplayAbilityEffectExtender(const TArray<FAssetData>& SelectedAssets);

} // namespace DcEditorExtra

#ifndef ATTRIBUTE_ACCESSORS
	#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

UCLASS()
class DATACONFIGEDITOREXTRA_API UDcTestAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UDcTestAttributeSet()
		: Health(1.0f)
		, Mana(0.0f)
	{}

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDcTestAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UDcTestAttributeSet, Mana)

};



