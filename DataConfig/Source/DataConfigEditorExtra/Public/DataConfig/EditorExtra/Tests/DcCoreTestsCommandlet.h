#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "DcCoreTestsCommandlet.generated.h"

UCLASS()
class UDcCoreTestsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:

	UDcCoreTestsCommandlet();

private:

	int32 Main(const FString& Params) override;
};


