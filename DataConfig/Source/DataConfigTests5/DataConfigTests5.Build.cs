using UnrealBuildTool;
using System.IO;

public class DataConfigTests5 : ModuleRules
{
	public DataConfigTests5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"DataConfigCore",
				"DataConfigExtra",
				"Core",
				"CoreUObject",
			});
	}
}
