#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DcDeserializeBPClassInstance.generated.h"

///	Deserialize Blueprint Sob Object Instance

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API EDcDeserializePredicateResult PredicateIsBPSubObjectProperty(FDcDeserializeContext& Ctx);
DATACONFIGEDITOREXTRA_API FDcResult HandlerBPSubObjectDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

} // namespace DcEditorExtra


UCLASS(Blueprintable, BlueprintType)
class DATACONFIGEDITOREXTRA_API UDcTestInstancedBPClassBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	FString GetClassID();
	FString GetClassID_Implementation();
};

USTRUCT()
struct DATACONFIGEDITOREXTRA_API FDcEditorExtraTestStructWithBPSubObject
{
	GENERATED_BODY()

	UPROPERTY() UDcTestInstancedBPClassBase* InstancedBaseField1;
	UPROPERTY() UDcTestInstancedBPClassBase* InstancedBaseField2;
	UPROPERTY() UDcTestInstancedBPClassBase* InstancedBaseField3;
};

