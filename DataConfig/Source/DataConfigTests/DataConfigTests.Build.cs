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
				//	ref: ShaderCompileWorker.Build.cs
				//	!! still need these so that it's linked
				"Core",
				"CoreUObject",
				"Projects",
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"Launch",
			});

		//	ref: UnrealHeaderTool.Build.cs
		//	ref: ShaderCompilerWorker.Build.cs
		string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		// For LaunchEngineLoop.cpp include
		PrivateIncludePaths.Add(
			EngineDir + "Source/Runtime/Launch/Private"
		);


	}
}
