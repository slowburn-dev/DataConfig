#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Commandlets/Commandlet.h"
#include "DcEditorExtraModule.generated.h"

class FDcEditorExtraModule : public IModuleInterface
{
public:
	static inline FDcEditorExtraModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcEditorExtraModule>("DataConfigEditorExtra");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigEditorExtra");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

UCLASS()
class UDcEditorExtraTestsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:

	UDcEditorExtraTestsCommandlet();

private:

	int32 Main(const FString& Params) override;
};


