#include "DcEditorExtraModule.h"

#include "ContentBrowserModule.h"
#include "Modules/ModuleManager.h"
#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "GameplayTagsManager.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/EditorExtra/Editor/DcEditorDumpAssetToLog.h"
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

	auto &ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    auto &ContextMenuExtenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	ContextMenuExtenders.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(DcEditorExtra::GameplayAbilityEffectExtender));
	ContextMenuExtenders.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(DcEditorExtra::DumpAssetToLogExtender));
	ContentExplorerExtenderHandlers.Add(ContextMenuExtenders.Last().GetHandle());
	ContentExplorerExtenderHandlers.Add(ContextMenuExtenders.Last().GetHandle());
}

void FDcEditorExtraModule::ShutdownModule()
{
	DcShutDown();
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module shutting down"));

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("DataConfig");

	if (ContentExplorerExtenderHandlers.Num() > 0 && FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		auto &ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		auto &ContextMenuExtenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		ContextMenuExtenders.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
		{
			return ContentExplorerExtenderHandlers.Contains( Delegate.GetHandle() );
		});

		ContentExplorerExtenderHandlers.Empty();
	}
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
