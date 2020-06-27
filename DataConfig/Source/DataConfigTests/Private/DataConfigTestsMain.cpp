#include "CoreMinimal.h"
#include "Misc/CompilationResult.h"
#include "Misc/ScopeExit.h"
#include "DcTypes.h"
#include "RequiredProgramMainCPPInclude.h"

#include "Adhocs.h"
IMPLEMENT_APPLICATION(DataConfigTests, "DataConfigTests");

static void Body()
{
	//PropertyVisitorRoundtrip();
	//JsonReader1();
	//UEStructSerializer();
	DeserializeSimple();

	return;
}

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

	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	FString ShortCmdLine = FCommandLine::RemoveExeName(*CmdLine);
	ShortCmdLine.TrimStartInline();

	if (GEngineLoop.PreInit(*ShortCmdLine) != 0)
	{
		UE_LOG(LogDataConfigCore, Error, TEXT("Failed to initialize the engine (PreInit failed)."));
		return ECompilationResult::CrashOrAssert;
	}

	Body();

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	return 0;
}
