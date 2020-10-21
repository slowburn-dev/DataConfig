#include "DataConfigEditorModule.h"
#include "ImportedDataAsset.h"
#include "AssetTypeActions_Base.h"
#include "EditorFramework/AssetImportData.h"
#include "MessageLogModule.h"
#include "Logging/MessageLog.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

IMPLEMENT_MODULE(FDataConfigEditorModule, DataConfigEditor);


struct FMessageLogDiagnosticConsumer : public DataConfig::IDcDiagnosticConsumer
{
	void HandleDiagnostic(DataConfig::FDcDiagnostic& Diag) override;
};

void FMessageLogDiagnosticConsumer::HandleDiagnostic(DataConfig::FDcDiagnostic& Diag)
{
	const FDcDiagnosticDetail* Detail = DcFindDiagnosticDetail(Diag.Code);
	if (Detail)
	{
		check(Detail->ID == Diag.Code.ErrorID);
		TArray<FStringFormatArg> FormatArgs;
		for (FDcDataVariant& Var : Diag.Args)
			FormatArgs.Add(DcConvertArg(Var));

		FMessageLog MessageLog("AssetReimport");
		MessageLog.Message(EMessageSeverity::Error, FText::FromString(FString::Format(Detail->Msg, FormatArgs)));
	}
	else
	{
		FMessageLog MessageLog("AssetReimport");
		MessageLog.Message(EMessageSeverity::Error, FText::FromString(TEXT("Unknown DataConfig Diagnostic")));
	}
}

class FAssetTypeActions_ImportedDataAsset : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ImportedDataAsset", "Imported Data Asset"); }
	FColor GetTypeColor() const override { return FColor(201, 29, 85); }
	uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	UClass* GetSupportedClass() const override { return UImportedDataAsset::StaticClass(); }
	bool IsImportedAsset() const override { return true; }

	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
};

void FAssetTypeActions_ImportedDataAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	//	pass for now
}

void FAssetTypeActions_ImportedDataAsset::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto ImportedDataAsset = CastChecked<UImportedDataAsset>(Asset);
		if (ImportedDataAsset->AssetImportData)
		{
			ImportedDataAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
		else
		{
			//	TODO report error
		}
	}
}

class FAssetTypeActions_PrimaryImportedDataAsset : public FAssetTypeActions_Base
{
public:
	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PrimaryImportedDataAsset", "Primary Imported Data Asset"); }
	FColor GetTypeColor() const override { return FColor(201, 29, 85); }
	uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	UClass* GetSupportedClass() const override { return UPrimaryImportedDataAsset::StaticClass(); }
	bool IsImportedAsset() const override { return true; }

	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
};

void FAssetTypeActions_PrimaryImportedDataAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	//	pass for now
}


void FAssetTypeActions_PrimaryImportedDataAsset::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto PrimaryImportedDataAsset = CastChecked<UPrimaryImportedDataAsset>(Asset);
		if (PrimaryImportedDataAsset->AssetImportData)
		{
			PrimaryImportedDataAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
		else
		{
			//	TODO report error
		}
	}
}

void FDataConfigEditorModule::StartupModule()
{
	DataConfig::DcStartUp();

	DataConfig::DcEnv().DiagConsumer = MakeShareable(new FMessageLogDiagnosticConsumer());

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	ImportedDataAssetActions.Emplace(MakeShareable(new FAssetTypeActions_ImportedDataAsset()));
	ImportedDataAssetActions.Emplace(MakeShareable(new FAssetTypeActions_PrimaryImportedDataAsset()));
	for (auto& ImportedDataAssetAction : ImportedDataAssetActions)
		AssetTools.RegisterAssetTypeActions(ImportedDataAssetAction.GetValue());

	//	we want to write into 'Asset Reimport' category in `Message Log` window
#define LOCTEXT_NAMESPACE "AutoReimportManager"
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	if (!MessageLogModule.IsRegisteredLogListing("AssetReimport"))
	{
		MessageLogModule.RegisterLogListing("AssetReimport", LOCTEXT("AssetReimportLabel", "Asset Reimport"));
	}
#undef LOCTEXT_NAMESPACE
}

void FDataConfigEditorModule::ShutdownModule()
{
	//	unregister
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& ImportedDataAssetAction : ImportedDataAssetActions)
		{
			check(ImportedDataAssetAction.IsSet());
			AssetToolsModule.UnregisterAssetTypeActions(ImportedDataAssetAction.GetValue());
		}
		ImportedDataAssetActions.Empty();
	}

	DataConfig::DcShutDown();
}

bool FDataConfigEditorModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

