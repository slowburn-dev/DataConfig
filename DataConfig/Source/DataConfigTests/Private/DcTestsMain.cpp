#include "CoreMinimal.h"
#include "Launch/Public/LaunchEngineLoop.h"
#include "Misc/ScopeExit.h"

#include "DataConfig/DcEnv.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"

static int32 TestRunnerBody()
{
	FDcAutomationConsoleRunner Runner;

	FDcAutomationConsoleRunner::FArgs Args;
	Args.Filters.Add(TEXT("DataConfig"));
	Args.RequestedTestFilter = FDcAutomationBase::_FLAGS;

	Runner.Prepare(Args);
	return Runner.RunTests();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	if (GEngineLoop.PreInit(ArgC, ArgV) != 0)
	{
		checkf(false, TEXT("Engine Preinit Failed"));
		return -1;
	}

	//	things printed before init is eaten
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	int32 RetCode;

	{
		DcRegisterDiagnosticGroup(&DcDExtra::Details);

		DcStartUp(EDcInitializeAction::SetAsConsole);
		RetCode = TestRunnerBody();
		DcShutDown();
	}

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	return RetCode;
}


