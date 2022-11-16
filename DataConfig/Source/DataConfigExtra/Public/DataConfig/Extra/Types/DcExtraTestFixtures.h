#pragma once

#include "CoreMinimal.h"
#include "DcExtraTestFixtures.generated.h"

USTRUCT()
struct FDcExtraTestSimpleStruct1
{
	GENERATED_BODY()

	UPROPERTY() FName NameField;
	UPROPERTY(meta = (DcSkip)) int IntFieldWithDefault = 253;
};

USTRUCT()
struct FDcExtraTestSimpleStruct2
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
};


DECLARE_DELEGATE(FDcExtraTestSingularAction);

USTRUCT()
struct FDcExtraTestDestructDelegateContainer
{
	GENERATED_BODY()

	FDcExtraTestSingularAction DestructAction;

	~FDcExtraTestDestructDelegateContainer()
	{
		DestructAction.ExecuteIfBound();
	}
};

USTRUCT()
struct FDcExtraTestCopyDelegateContainer
{
	GENERATED_BODY()

	UPROPERTY() FString PropValue;
	FString NonPropValue;

	FDcExtraTestSingularAction CopyAction;

	FDcExtraTestCopyDelegateContainer() = default;

	FDcExtraTestCopyDelegateContainer& operator=(const FDcExtraTestCopyDelegateContainer& InOther)
	{
		if (this != &InOther)
		{
			InOther.CopyAction.ExecuteIfBound();
			PropValue = InOther.PropValue;
			NonPropValue = InOther.NonPropValue;
		}
	
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) FName ShapeName;
};

USTRUCT(BlueprintType)
struct FDcStructShapeRectangle : public FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) float Height;
	UPROPERTY(EditAnywhere) float Width;
};

USTRUCT(BlueprintType)
struct FDcStructShapeCircle : public FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) float Radius;
};

