// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

//  ! this is a horrible hack to make UBT projectfiles generation pass        
public class DataConfigHeadlessTarget : TargetRules
{
	public DataConfigHeadlessTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		Type = TargetType.Editor;
		LaunchModuleName = "DataConfigHeadless";
	}
}
