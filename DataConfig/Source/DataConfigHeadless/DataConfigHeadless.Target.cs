using UnrealBuildTool;
using System.Collections.Generic;

public class DataConfigHeadlessTarget : TargetRules
{
	public DataConfigHeadlessTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		
		Type = TargetType.Program;
		LaunchModuleName = "DataConfigHeadless";
		//	! note that only monolithic works atm
		LinkType = TargetLinkType.Monolithic;

		MinGameModuleSourceFilesForUnityBuild = 2;
		bUseMallocProfiler = false;

		//	! need this for logging
        bUseLoggingInShipping = true;

        bCompileICU = false;

		bCompileAgainstEngine = false;
		bCompileAgainstCoreUObject = true;
		bCompileAgainstApplicationCore = true;

		bForceEnableExceptions = false;

		bCompileWithPluginSupport = false;
		bBuildDeveloperTools = false;

		bIsBuildingConsoleApplication = true;

#if UE_5_1_OR_LATER
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

		//	toggle to use clang-cl
		//	it works mostly except breaks FName natvis
		// WindowsPlatform.Compiler = WindowsCompiler.Clang;

		//	toggle to buld with no `WITH_EDITORONLY_DATA`
		// bBuildWithEditorOnlyData = false;
		bBuildWithEditorOnlyData = true;
	}
}
