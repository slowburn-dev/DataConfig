// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DataConfigTestsTarget : TargetRules
{
	public DataConfigTestsTarget(TargetInfo Target) : base(Target)
	{
		//	ref: UnrealHeaderTool.Target.cs
		DefaultBuildSettings = BuildSettingsVersion.V2;
		
		Type = TargetType.Program;
		// LinkType = TargetLinkType.Modular;
		//	mono link also works, use modular link just to repor symbol exports
		LinkType = TargetLinkType.Monolithic;
		LaunchModuleName = "DataConfigTests";

		bUseMallocProfiler = false;

        bCompileICU = false;
        // Editor-only data, however, is needed
		bBuildWithEditorOnlyData = true;

		//	this actuall defines WITH_ENGINE
		bCompileAgainstEngine = false;
		//	need these 2 to link Engine
		bCompileAgainstCoreUObject = true;
		bCompileAgainstApplicationCore = true;

		bForceEnableExceptions = false;

		// no plugin for this
		bCompileWithPluginSupport = false;
		bBuildDeveloperTools = false;

		bIsBuildingConsoleApplication = true;
	}
}
