#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DcTestDeserializeEditor.generated.h"

UCLASS()
class DATACONFIGEDITOREXTRA_API UDcEditorExtraNativeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY() FString StrField;
	UPROPERTY() int IntField;
};

USTRUCT()
struct FDcEditorExtraTestObjectRefs1
{
	GENERATED_BODY()

	UPROPERTY() UObject* ObjField1;
	UPROPERTY() UObject* ObjField2;
	UPROPERTY() UObject* ObjField3;
	UPROPERTY() UObject* ObjField4;
};




