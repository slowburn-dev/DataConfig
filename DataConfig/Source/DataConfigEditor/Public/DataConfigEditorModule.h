#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;

struct FDataConfigEditorModule
	: public IModuleInterface
	, FSelfRegisteringExec
{
	void StartupModule() override;
	void ShutdownModule() override;

	//~ FSelfRegisteringExec interface
	bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	TArray<TOptional<TSharedRef<FAssetTypeActions_Base>>> ImportedDataAssetActions;
};





