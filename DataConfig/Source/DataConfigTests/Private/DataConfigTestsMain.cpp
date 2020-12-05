#include "CoreMinimal.h"
#include "Misc/CompilationResult.h"
#include "Misc/ScopeExit.h"
#include "DataConfig/DcTypes.h"
#include "RequiredProgramMainCPPInclude.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "GenericPlatform/GenericApplication.h"


#include "Adhocs.h"
IMPLEMENT_APPLICATION(DataConfigTests, "DataConfigTests");

static void Body()
{
	//	Pre Init
	//	Seesm plugin is disable for Exe build types
	/*
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("DataConfig");
	if (!Plugin
		|| !Plugin->IsEnabled())
	{
		checkNoEntry();
		return;
	}
	*/

	//	this is adhoc anyway, figure this out later
	FPackageName::RegisterMountPoint(
		TEXT("/Fixture/"),
		TEXT("C:/DevUE/projects/DataConfig/DataConfig/DataConfig/Content/Fixture")
	);

	//	TODO this doesn't work yet
	//WriteFixtureAsset();

	/*
	PropertyVisitorRoundtrip_Piped();
	PropertyVisitorRoundtrip__TestObjAlpha();
	PropertyVisitorRoundtrip__StructStruct();
	PropertyVisitorRoundtrip__StructHasMap();
	PropertyVisitorRoundtrip_ReadNested();
	PropertyVisitorRoundtrip_WriteNested();
	PropertyVisitorRoundtrip();
	UEStructSerializer();

	DeserializeSimple();
	DeserializeNestedStruct();
	DeserializeObjectRoot();
	DeserializeObjectRef();
	DeserializeSubObject();

	TryOutVariant();
	TryOutPutback();
	//TryDiags();
	//TryConsoleDiagnosticReports();

	SourceTypes();
	JsonReader1();
	JsonCanada();

	PropertyVisitorRoundtrip__Basic();
	PropertyVisitorRoundtrip__MapStruct();
	PropertyVisitorRoundtrip__StructStruct();
	TryTemplates();

	PropertyVisitorRoundtrip__Basic();
	//PropertyVisitorRoundtrip__MapStruct();
	PropertyVisitorRoundtrip__StructStruct();
	PropertyVisitorRoundtrip__Enum();
	PropertyVisitorRoundtrip__Text();
	PropertyVisitorRoundtrip__ClassReference();
	PropertyVisitorRoundtrip__Objects();

	PropertyVisitorRoundtrip__ScriptStruct();
	PropertyVisitorRoundtrip_SoftWeakLazy();
	PropertyVisitorRoundtrip_ScriptInterface();
	PropertyVisitorRoundtrip_Delegates();
	//JsonFail1();
	//DownwardFromHere();
	PropertyReadWrite_Blob();

	DeserializeExtra_Color();
	*/

	DeserializeExtra_Color();

	return;
}

#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
static int32 _DumpStackAndExit(Windows::LPEXCEPTION_POINTERS ExceptionInfo)
{
	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)FMemory::SystemMalloc(StackTraceSize);

	StackTrace[0] = 0;
	FPlatformStackWalk::StackWalkAndDumpEx(
		StackTrace,
		StackTraceSize, 
		7,
		FGenericPlatformStackWalk::EStackWalkFlags::FlagsUsedWhenHandlingEnsure);

	puts(StackTrace);
	FMemory::SystemFree(StackTrace);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif


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

	//	crash handler
	//	windows literally doesn't have this funciton implemented, using SEH
	/*
	FPlatformMisc::SetCrashHandler([](const FGenericCrashContext& Context) {});
	*/
	FPlatformMisc::SetGracefulTerminationHandler();

	if (GEngineLoop.PreInit(*ShortCmdLine) != 0)
	{
		UE_LOG(LogDataConfigCore, Error, TEXT("Failed to initialize the engine (PreInit failed)."));
		return ECompilationResult::CrashOrAssert;
	}

#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
	__try
#endif

	{
		DcStartUp(EDcInitializeAction::SetAsConsole);
		Body();
		DcShutDown();
	}

#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
	__except (_DumpStackAndExit(GetExceptionInformation()))
	{
		FPlatformMisc::RequestExit(true);
	}
#endif

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	return 0;
}
