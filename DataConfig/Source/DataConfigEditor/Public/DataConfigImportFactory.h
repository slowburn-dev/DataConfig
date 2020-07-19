#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "DataConfigImportFactory.generated.h"

//	TODO implement IImportSettingsParser to support everything
UCLASS(hidecategories = Object)
class DATACONFIGEDITOR_API UDataConfigImportFactory : public UFactory, public FReimportHandler 
{
	GENERATED_BODY()

	UDataConfigImportFactory();

	//~ Begin UFactory Interface
	FText GetDisplayName() const override;
	UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	bool DoesSupportClass(UClass* Class) override;
	bool FactoryCanImport(const FString& Filename) override;

	//~ Begin FReimporthandler Interface
	bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	EReimportResult::Type Reimport(UObject* Obj) override;
	int32 GetPriority() const override;
};

