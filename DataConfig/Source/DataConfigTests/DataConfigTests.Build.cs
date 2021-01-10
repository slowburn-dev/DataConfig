using UnrealBuildTool;
using System.IO;

public class DataConfigTests : ModuleRules
{
	public DataConfigTests(ReadOnlyTargetRules Target) : base(Target)
	{
	    bRequiresImplementModule = false;
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
