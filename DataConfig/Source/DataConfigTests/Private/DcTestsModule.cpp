#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"

class FDcTestsModule : public IModuleInterface
{
public:
	static inline FDcTestsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcTestsModule>("DataConfigTests");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigTests");
	}

	void StartupModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcTestsModule module starting up"));
	}

	void ShutdownModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcTestsModule module shutting down"));
	}
};

IMPLEMENT_MODULE(FDcTestsModule, DataConfigTests);

