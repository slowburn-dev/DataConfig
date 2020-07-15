#include "DataConfigEditorModule.h"

IMPLEMENT_MODULE(FDataConfigEditorModule, DataConfigEditor);

void FDataConfigEditorModule::StartupModule()
{

}

void FDataConfigEditorModule::ShutdownModule()
{

}

bool FDataConfigEditorModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

