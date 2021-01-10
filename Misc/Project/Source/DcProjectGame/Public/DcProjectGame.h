#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDcProjectGame, All, All);

class FDcProjectGame : public IModuleInterface
{
public:
	static inline FDcProjectGame& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcProjectGame>("DcProjectGame");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DcProjectGame");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
