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

		//	note that modular link doesn't work for this target as DC_TEST() registration
		//	fails to load from other dlls
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
	}
}
