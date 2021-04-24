#include "CoreMinimal.h"
#include "Misc/CommandLine.h"
#include "RequiredProgramMainCPPInclude.h"

#include "DataConfig/DcVersion.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"

///
/// Usage:
///	DataConfigHeadless [TestFilter1] [TestFilter2] ...
///

IMPLEMENT_APPLICATION(DataConfigHeadless, "DataConfigHeadless");


static int32 TestRunnerBody(TArray<FString>& Tokens)
{
	FDcAutomationConsoleRunner Runner;

	FDcAutomationConsoleRunner::FArgs Args;
	Args.Filters.Add(TEXT("DataConfig"));

	int ParametersSplitIx = -1;
	//	note that we look for "--", but FCommandLine::Parse eats one '-'
	bool bHasParametersSplit = Tokens.Find(TEXT("-"), ParametersSplitIx);
	if (bHasParametersSplit)
	{
		for (int Ix = 0; Ix < ParametersSplitIx; Ix++)
			Args.Filters.Add(Tokens[Ix]);

		for (int Ix = ParametersSplitIx+1; Ix < Tokens.Num(); Ix++)
			Args.Parameters.Add(Tokens[Ix]);
	}
	else
	{
		for (FString& Token : Tokens)
			Args.Filters.Add(Token);
	}

	Args.RequestedTestFilter = FDcAutomationBase::FLAGS;

	Runner.Prepare(Args);
	return Runner.RunTests();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	if (GEngineLoop.PreInit(ArgC, ArgV) != 0) // NOLINT
	{
		checkf(false, TEXT("Engine Preinit Failed"));
		return -1;
	}

	TArray<FString> Tokens;
	TArray<FString> Switches;
	{
		FString CmdLine;
		for (int32 Arg = 0; Arg < ArgC; Arg++)
		{
			FString LocalArg = ArgV[Arg];
			if (LocalArg.Contains(TEXT(" "), ESearchCase::CaseSensitive))
			{
				CmdLine += TEXT("\"");
				CmdLine += LocalArg;
				CmdLine += TEXT("\"");
			}
			else
			{
				CmdLine += LocalArg;
			}

			if (Arg + 1 < ArgC)
			{
				CmdLine += TEXT(" ");
			}
		}

		FString ShortCmdLine = FCommandLine::RemoveExeName(*CmdLine);
		ShortCmdLine.TrimStartInline();

		FCommandLine::Parse(*ShortCmdLine, Tokens, Switches);
	}

	UE_LOG(LogDataConfigCore, Display, TEXT("DataConfigCore Version: %s, %d"),
		TEXT(DATA_CONFIG_CORE_VERSION),
		DATA_CONFIG_CORE_VERSION_NUMBER
	);

	int32 RetCode;
	{
		DcRegisterDiagnosticGroup(&DcDExtra::Details);

		DcStartUp(EDcInitializeAction::SetAsConsole);
		RetCode = TestRunnerBody(Tokens);
		DcShutDown();
	}

	RequestEngineExit(TEXT("DataConfigHeadless Main Exit"));
	FEngineLoop::AppPreExit();
	FEngineLoop::AppExit();

	return RetCode;
}


