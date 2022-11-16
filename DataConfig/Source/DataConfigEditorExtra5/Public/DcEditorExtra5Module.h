#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

class FDcEditorExtra5Module : public IModuleInterface
{
public:
	FORCEINLINE static FDcEditorExtra5Module& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcEditorExtra5Module>("DataConfigEditorExtra5");
	}

	FORCEINLINE static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigEditorExtra5");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
