#include "CoreMinimal.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/DcTypes.h"
#include "Launch/Public/LaunchEngineLoop.h"
#include "Misc/ScopeExit.h"

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	if (GEngineLoop.PreInit(ArgC, ArgV) != 0)
	{
		checkf(false, TEXT("Engine Preinit Failed"));
		return -1;
	}

	//	things printed before init is eaten
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	FDcAutomationConsoleRunner Runner;

	Runner.Prepare();
	Runner.RunTests();

	FAutomationTestFramework::Get().SetForceSmokeTests(true);
	FAutomationTestFramework::Get().RunSmokeTests();

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	return 0;
}


