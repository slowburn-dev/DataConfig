#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"

class FDcTests54Module : public IModuleInterface
{
public:
	static inline FDcTests54Module& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcTests54Module>("DataConfigTests5");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigTests54");
	}

	void StartupModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("FDcTests54Module module starting up"));
	}

	void ShutdownModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("FDcTests54Module module shutting down"));
	}
};

IMPLEMENT_MODULE(FDcTests54Module, DataConfigTests54);

