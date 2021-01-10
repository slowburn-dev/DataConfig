#include "DcEditorExtraModule.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"

void FDcEditorExtraModule::StartupModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module starting up"));
	DcStartUp(EDcInitializeAction::SetAsConsole);
}

void FDcEditorExtraModule::ShutdownModule()
{
	DcShutDown();
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module shutting down"));
}

IMPLEMENT_MODULE(FDcEditorExtraModule, DataConfigEditorExtra);
