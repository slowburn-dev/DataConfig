using UnrealBuildTool;
using System.IO;

public class DataConfigTests : ModuleRules
{
	public DataConfigTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"DataConfigCore",
				"DataConfigExtra",
				//	ref: ShaderCompileWorker.Build.cs
				//	!! still need these so that it's linked
				"Core",
				"CoreUObject",
				"ApplicationCore",
				"Projects",
				//	try out ue4 serialization
				"Json",
				"Serialization",
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"Launch",
			});

		//	seems `TestPal` is an up to date console application example
		/*
		//	ref: UnrealHeaderTool.Build.cs
		//	ref: ShaderCompilerWorker.Build.cs
		string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		// For LaunchEngineLoop.cpp include
		PrivateIncludePaths.Add(
			EngineDir + "Source/Runtime/Launch/Private"
		);
		*/


	}
}
