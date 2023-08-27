#pragma once

#include "CoreMinimal.h"
#include "DcExtraTestFixtures.generated.h"

USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraTestSimpleStruct1
{
	GENERATED_BODY()

	UPROPERTY() FName NameField;
	UPROPERTY(meta = (DcSkip)) int IntFieldWithDefault = 253;
};

USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraTestSimpleStruct2
{
	GENERATED_BODY()

	UPROPERTY() FString StrField;
};


DECLARE_DELEGATE(FDcExtraTestSingularAction);

USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraTestDestructDelegateContainer
{
	GENERATED_BODY()

	FDcExtraTestSingularAction DestructAction;

	~FDcExtraTestDestructDelegateContainer()
	{
		DestructAction.ExecuteIfBound();
	}
};

USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraTestCopyDelegateContainer
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
struct DATACONFIGEXTRA_API FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) FName ShapeName;
};

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcStructShapeRectangle : public FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) float Height;
	UPROPERTY(EditAnywhere) float Width;
};

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcStructShapeCircle : public FDcStructShapeBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) float Radius;
};


UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class DATACONFIGEXTRA_API UDcBaseShape : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) FName ShapeName;
};

UCLASS()
class DATACONFIGEXTRA_API UDcShapeBox : public UDcBaseShape
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) float Height;
	UPROPERTY(EditAnywhere) float Width;
};

UCLASS()
class DATACONFIGEXTRA_API UDcShapeSquare : public UDcBaseShape
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere) float Radius;
};


UENUM()
enum class EDcExtraTestEnum1
{
	Alpha,
	Beta,
	Gamma,
};

USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraSimpleStruct
{
    GENERATED_BODY()

    UPROPERTY() FString Name;
    UPROPERTY() int Id;
    UPROPERTY() EDcExtraTestEnum1 Type;
};


