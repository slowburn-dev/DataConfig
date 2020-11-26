#include "CoreMinimal.h"
#include "Misc/CompilationResult.h"
#include "Misc/ScopeExit.h"
#include "DataConfig/DcTypes.h"
#include "RequiredProgramMainCPPInclude.h"
#include "Interfaces/IPluginManager.h"


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
	TryDiags();
	TryConsoleDiagnosticReports();

	SourceTypes();
	JsonReader1();

	JsonFail1();
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
	PropertyVisitorRoundtrip__Class();

	//PropertyVisitorRoundtrip__ScriptStruct();
	//JsonFail1();

	PropertyVisitorRoundtrip__ScriptStruct();

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

	DcStartUp(EDcInitializeAction::SetAsConsole);
	Body();
	DcShutDown();

	ON_SCOPE_EXIT
	{
		FEngineLoop::AppPreExit();
		FEngineLoop::AppExit();
	};

	return 0;
}
