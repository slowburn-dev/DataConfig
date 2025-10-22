#pragma once

#include "CoreMinimal.h"
#include "DcTestProperty5.generated.h"

USTRUCT()
struct FDcTestStructWithDefaults
{
	GENERATED_BODY()

	enum { _DEFAULT_INT = 254 };

	UPROPERTY() FString StrFieldWithDefault = "Foo";
	UPROPERTY() int IntFieldWithDefault = _DEFAULT_INT;

	UPROPERTY() TArray<FString> StrArrFieldWithDefault = {"Foo", "Bar", "Baz"};
	UPROPERTY() TSet<FString> StrSetFieldWithDefault = {"One", "Two", "Three"};
	UPROPERTY() TMap<FString, int> StringIntMapFieldWithDefault = {
		{"One", 1},
		{"Two", 2},
		{"Three", 1 + 1 + 1 * 1},
	};
};

USTRUCT()
struct FDcTestStructWithText
{
	GENERATED_BODY()

	UPROPERTY() FText TextFieldAlpha;
	UPROPERTY() FText TextFieldBeta;

	void MakeFixture();
};
