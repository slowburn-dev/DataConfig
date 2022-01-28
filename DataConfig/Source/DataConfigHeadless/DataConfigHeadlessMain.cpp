#include "CoreMinimal.h"
#include "Misc/CommandLine.h"
#include "RequiredProgramMainCPPInclude.h"

#include "DataConfig/DcVersion.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"

#include "stdio.h"

///
/// Usage:
///	DataConfigHeadless [TestFilter1] [TestFilter2] ...
///

IMPLEMENT_APPLICATION(DataConfigHeadless, "DataConfigHeadless");


static int32 TestRunnerBody(TArray<FString>& Tokens)
{
	FDcAutomationConsoleRunner Runner;
	Runner.Prepare(FDcAutomationConsoleRunner::FromCommandlineTokens(Tokens));
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

	if (Switches.Contains(TEXT("WaitInput")))
	{
		Tokens.Remove(TEXT("WaitInput"));

		UE_LOG(LogDataConfigCore, Display, TEXT("!!! detected WaitInput, input anything to continue "));
		getchar();
	}

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


