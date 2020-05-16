#include "CoreMinimal.h"
#include "Misc/ScopeExit.h"
#include "DataConfigTypes.h"

#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(DataConfigTests, "DataConfigTests");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	FString CmdLine;

	for (int32 Arg = 0; Arg < ArgC; Arg++)
	{
		FString LocalArg = ArgV[Arg];
		if (LocalArg.Contains(TEXT(" ")))
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

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	Hello();

	return 0;
}
