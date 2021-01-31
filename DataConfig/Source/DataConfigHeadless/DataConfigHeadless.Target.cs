// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

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

        bCompileICU = false;
		bBuildWithEditorOnlyData = true;

		bCompileAgainstEngine = false;
		bCompileAgainstCoreUObject = true;
		bCompileAgainstApplicationCore = true;

		bForceEnableExceptions = false;

		bCompileWithPluginSupport = false;
		bBuildDeveloperTools = false;

		bIsBuildingConsoleApplication = true;

		//	toggle to use clang-cl
		//	it works mostly except breaks FName natvis
		WindowsPlatform.Compiler = WindowsCompiler.Clang;
	}
}
