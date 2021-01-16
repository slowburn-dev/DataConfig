#include "DcEditorExtraModule.h"
#include "Modules/ModuleManager.h"
#include "MessageLogModule.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"

struct FDcMessageLogDiagnosticConsumer : public IDcDiagnosticConsumer
{
	void HandleDiagnostic(FDcDiagnostic& Diag) override
	{
		const FDcDiagnosticDetail* Detail = DcFindDiagnosticDetail(Diag.Code);
		if (Detail)
		{
			check(Detail->ID == Diag.Code.ErrorID);
			TArray<FStringFormatArg> FormatArgs;
			for (FDcDataVariant& Var : Diag.Args)
				FormatArgs.Add(DcConvertArg(Var));

			FString OutMessage;
			OutMessage.Append(TEXT("# DataConfig Error: "));
			OutMessage.Append(FString::Format(Detail->Msg, FormatArgs));
			if (Diag.Highlights.Num())
			{
				for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
				{
					OutMessage.AppendChar(TCHAR('\n'));
					if (Highlight.FileContext.IsSet())
					{
						OutMessage.Appendf(TEXT("- [%s] --> %s%d:%d\n%s"),
							*Highlight.OwnerName,
							*Highlight.FileContext->FilePath,
							Highlight.FileContext->Loc.Line,
							Highlight.FileContext->Loc.Column,
							*Highlight.Formatted
						);
					}
					else
					{
						OutMessage.Appendf(TEXT("- [%s] %s"),
							*Highlight.OwnerName,
							*Highlight.Formatted
						);
					}
				}
			}

			FMessageLog MessageLog("DataConfig");
			MessageLog.Message(EMessageSeverity::Error, FText::FromString(OutMessage));
		}
		else
		{
			FMessageLog MessageLog("DataConfig");
			MessageLog.Message(EMessageSeverity::Error, FText::FromString(TEXT("Unknown DataConfig Diagnostic")));
		}
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
}

void FDcEditorExtraModule::ShutdownModule()
{
	DcShutDown();
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module shutting down"));

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("DataConfig");
}

IMPLEMENT_MODULE(FDcEditorExtraModule, DataConfigEditorExtra);
