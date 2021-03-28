#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"

class FDcExtraModule : public IModuleInterface
{
public:
	FORCEINLINE static FDcExtraModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FDcExtraModule>("DataConfigExtra");
	}

	FORCEINLINE static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DataConfigExtra");
	}

	void StartupModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcExtraModule module starting up"));
	}

	void ShutdownModule() override
	{
		UE_LOG(LogDataConfigCore, Log, TEXT("DcExtraModule module starting up"));
	}
};

IMPLEMENT_MODULE(FDcExtraModule, DataConfigExtra);

