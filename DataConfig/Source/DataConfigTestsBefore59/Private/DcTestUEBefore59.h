#pragma once

#include "CoreMinimal.h"
#include "DcTestUEBefore59.generated.h"

UCLASS()
class UDcTestBefore59Object : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UDcTestLazyObjectRefs : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY() TLazyObjectPtr<UObject> LazyObjectField;
	UPROPERTY() TLazyObjectPtr<UObject> LazyObjectArr[3];
	UPROPERTY() TLazyObjectPtr<UDcTestBefore59Object> TestClassLazyObjectField;
};

USTRUCT()
struct FDcTestLazyObjectSerDe
{
	GENERATED_BODY()

	UPROPERTY() TSoftObjectPtr<UObject> SoftField1;
	UPROPERTY() TSoftObjectPtr<UObject> SoftField2;
	UPROPERTY() TLazyObjectPtr<UObject> LazyField1;
	UPROPERTY() TLazyObjectPtr<UObject> LazyField2;

	void MakeFixture();
};
