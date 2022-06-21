#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty.h"
#include "DcTestProperty3.generated.h"


USTRUCT()
struct FDcTestMeta1
{
	GENERATED_BODY()

	UPROPERTY(meta = (DcSkip)) int SkipField1;
};


USTRUCT()
struct FDcTestClassAccessControl
{
	GENERATED_BODY()

	UPROPERTY() UDcBaseShape* InlineField;

	UPROPERTY() UDcTestClass1* RefField;
};
