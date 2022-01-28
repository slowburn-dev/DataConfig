#pragma once

#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "Engine/DataAsset.h"
#include "DcSerDeBlueprint.generated.h"

///	Blueprint SerDe, including:
/// - Blueprint class/object reference
/// - Blueprint class instances
/// - Blueprint struct with FDcAnyStruct
/// - Blueprint enum

namespace DcEditorExtra {

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPClassReferenceDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPEnumDeserialize(FDcDeserializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPClassReferenceSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPObjectReferenceSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPEnumSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPStructSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPDcAnyStructSerialize(FDcSerializeContext& Ctx);

DATACONFIGEDITOREXTRA_API FDcResult HandlerBPStructRootSerializeSkipEmptyAnyStruct(FDcSerializeContext& Ctx);

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

