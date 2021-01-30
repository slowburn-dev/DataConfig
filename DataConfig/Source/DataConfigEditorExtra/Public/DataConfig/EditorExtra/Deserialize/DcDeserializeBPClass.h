#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "Engine/DataAsset.h"
#include "DcDeserializeBPClass.generated.h"

///	Deserialize Blueprint Sob Object Instance

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPClassReferenceDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPDcAnyStructDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);

} // namespace DcEditorExtra


UCLASS(Blueprintable, BlueprintType)
class DATACONFIGEDITOREXTRA_API UDcTestBPClassBase : public UDataAsset
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	FString GetClassID();
	FString GetClassID_Implementation();
};

UCLASS()
class DATACONFIGEDITOREXTRA_API UDcTestNativeDerived1 : public UDcTestBPClassBase
{
	GENERATED_BODY()
};


USTRUCT()
struct DATACONFIGEDITOREXTRA_API FDcEditorExtraTestStructWithBPClass
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<UDcTestBPClassBase> ClassField1;
	UPROPERTY() TSubclassOf<UDcTestBPClassBase> ClassField2;
	UPROPERTY() TSubclassOf<UDcTestBPClassBase> ClassField3;
};

USTRUCT()
struct DATACONFIGEDITOREXTRA_API FDcEditorExtraTestStructWithBPInstance
{
	GENERATED_BODY()

	UPROPERTY() UDcTestBPClassBase* InstanceField1;
	UPROPERTY() UDcTestBPClassBase* InstanceField2;
	UPROPERTY() UDcTestBPClassBase* InstanceField3;
};

