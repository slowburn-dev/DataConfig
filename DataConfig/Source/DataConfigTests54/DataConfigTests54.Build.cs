using UnrealBuildTool;
using System.IO;

public class DataConfigTests54 : ModuleRules
{
	public DataConfigTests54(ReadOnlyTargetRules Target) : base(Target)
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
