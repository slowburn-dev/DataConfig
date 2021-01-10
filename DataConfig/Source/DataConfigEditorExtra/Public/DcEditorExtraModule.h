#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

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





