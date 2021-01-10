#include "DcEditorExtraModule.h"
#include "Modules/ModuleManager.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"

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

			FMessageLog MessageLog("DataConfig");
			MessageLog.Message(EMessageSeverity::Error, FText::FromString(FString::Format(Detail->Msg, FormatArgs)));

			for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
			{
				FString HighlightHeader;
				if (Highlight.FileContext.IsSet())
				{
					HighlightHeader = FString::Printf(TEXT("File: %s%d:%d"),
						*Highlight.FileContext->FilePath,
						Highlight.FileContext->Loc.Line,
						Highlight.FileContext->Loc.Column);
				}
				else
				{
					HighlightHeader = TEXT("<Unkown file>");
				}

				if (!Highlight.Formatted.IsEmpty())
				{
					MessageLog.Message(EMessageSeverity::Error, FText::FromString(
						FString::Printf(TEXT("%s\n%s"), *HighlightHeader, *Highlight.Formatted)
					));
				}
			}
		}
		else
		{
			FMessageLog MessageLog("DataConfig");
			MessageLog.Message(EMessageSeverity::Error, FText::FromString(TEXT("Unknown DataConfig Diagnostic")));
		}
	}
};


void FDcEditorExtraModule::StartupModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module starting up"));
	DcStartUp(EDcInitializeAction::Minimal);
	DcEnv().DiagConsumer = MakeShareable(new FDcMessageLogDiagnosticConsumer());
}

void FDcEditorExtraModule::ShutdownModule()
{
	DcShutDown();
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module shutting down"));
}

IMPLEMENT_MODULE(FDcEditorExtraModule, DataConfigEditorExtra);
