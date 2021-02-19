#include "DcEditorExtraModule.h"

#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "GameplayTagsManager.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"
#include "DataConfig/Automation/DcAutomation.h"

struct FDcMessageLogDiagnosticConsumer : public IDcDiagnosticConsumer
{
	void HandleDiagnostic(FDcDiagnostic& Diag) override
	{
		FMessageLog MessageLog("DataConfig");
		MessageLog.Message(EMessageSeverity::Error, FText::FromString(DcDiagnosticToString(Diag)));
	}
};

static void _PopulateEditorExtraGameplayTagFixtures()
{
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Foo.Bar"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Tar.Taz"));
}

void FDcEditorExtraModule::StartupModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module starting up"));
	DcRegisterDiagnosticGroup(&DcDExtra::Details);
	DcRegisterDiagnosticGroup(&DcDEditorExtra::Details);

	DcStartUp(EDcInitializeAction::Minimal);
	DcEnv().DiagConsumer = MakeShareable(new FDcMessageLogDiagnosticConsumer());

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bAllowClear = true;
	MessageLogModule.RegisterLogListing("DataConfig", FText::FromString(TEXT("DataConfig")), InitOptions);

	_PopulateEditorExtraGameplayTagFixtures();

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetActions.Emplace(MakeShareable(new DcEditorExtra::FAssetTypeActions_DcGameplayAbility()));
	AssetActions.Emplace(MakeShareable(new DcEditorExtra::FAssetTypeActions_DcGameplayEffect()));
	for (auto& ImportedDataAssetAction : AssetActions)
		AssetTools.RegisterAssetTypeActions(ImportedDataAssetAction.GetValue());
}

void FDcEditorExtraModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& ImportedDataAssetAction : AssetActions)
		{
			check(ImportedDataAssetAction.IsSet());
			AssetToolsModule.UnregisterAssetTypeActions(ImportedDataAssetAction.GetValue());
		}
		AssetActions.Empty();
	}

	DcShutDown();
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module shutting down"));

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("DataConfig");
}

IMPLEMENT_MODULE(FDcEditorExtraModule, DataConfigEditorExtra);

UDcEditorExtraTestsCommandlet::UDcEditorExtraTestsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UDcEditorExtraTestsCommandlet::Main(const FString& Params)
{
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);
	UE_LOG(LogDataConfigCore, Display, TEXT("================================================================="));

	TArray<FString> Tokens;
	TArray<FString> Switches;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches);
	Tokens.RemoveAt(0); // 0 is always commandlet name

	FDcAutomationConsoleRunner Runner;

	FDcAutomationConsoleRunner::FArgs Args;
	Args.Filters.Add(TEXT("DataConfig"));
	for (FString& Token : Tokens)
		Args.Filters.Add(Token);

	Args.RequestedTestFilter = FDcAutomationBase::FLAGS;

	Runner.Prepare(Args);
	int32 Ret = Runner.RunTests();

	UE_LOG(LogDataConfigCore, Display, TEXT("================================================================="));

	return Ret;
}
