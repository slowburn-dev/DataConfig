#include "DcEditorExtra5Module.h"
#include "DataConfig/DcTypes.h"

void FDcEditorExtra5Module::StartupModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DataConfigEditorExtra5 module starting up"));
}

void FDcEditorExtra5Module::ShutdownModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DataConfigEditorExtra5 module shutting down"));
}

IMPLEMENT_MODULE(FDcEditorExtra5Module, DataConfigEditorExtra5);
