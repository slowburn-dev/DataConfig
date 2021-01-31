#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

class FDcProjectEditor : public IModuleInterface
{
public:
	static inline FDcProjectEditor& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcProjectEditor>("DcProjectEditor");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DcProjectEditor");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
