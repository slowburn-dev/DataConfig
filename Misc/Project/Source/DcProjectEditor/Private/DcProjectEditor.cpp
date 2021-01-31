#include "DcProjectEditor.h"
#include "DcProjectGame.h"
#include "Modules/ModuleManager.h"

void FDcProjectEditor::StartupModule()
{
	UE_LOG(LogDcProjectGame, Log, TEXT("DcProjectEditor module starting up"));
}

void FDcProjectEditor::ShutdownModule()
{
	UE_LOG(LogDcProjectGame, Log, TEXT("DcProjectEditor module shutting down"));
}

IMPLEMENT_MODULE(FDcProjectEditor, DcProjectEditor);
