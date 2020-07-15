#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


struct FDataConfigEditorModule
	: public IModuleInterface
	, FSelfRegisteringExec
{
	void StartupModule() override;
	void ShutdownModule() override;

	//~ FSelfRegisteringExec interface
	bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
};


