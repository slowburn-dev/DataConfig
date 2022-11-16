#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"

class FDcTests5Module : public IModuleInterface
{
public:
	static inline FDcTests5Module& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcTests5Module>("DataConfigTests5");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigTests5");
	}

	void StartupModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcTests5Module module starting up"));
	}

	void ShutdownModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcTests5Module module shutting down"));
	}
};

IMPLEMENT_MODULE(FDcTests5Module, DataConfigTests5);

