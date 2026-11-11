#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"

class FDcTestsBefore59Module : public IModuleInterface
{
public:
	static inline FDcTestsBefore59Module& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcTestsBefore59Module>("DataConfigTestsBefore59");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigTestsBefore59");
	}

	void StartupModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("FDcTestsBefore59Module module starting up"));
	}

	void ShutdownModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("FDcTestsBefore59Module module shutting down"));
	}
};

IMPLEMENT_MODULE(FDcTestsBefore59Module, DataConfigTestsBefore59);
