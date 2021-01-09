// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DataConfigCoreTarget : TargetRules
{
	public DataConfigCoreTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		// ExtraModuleNames.AddRange( new string[] { "SampleProject" } );
	}
}
