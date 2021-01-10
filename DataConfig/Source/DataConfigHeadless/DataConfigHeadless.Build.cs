using UnrealBuildTool;
using System.IO;

public class DataConfigHeadless : ModuleRules
{
	public DataConfigHeadless(ReadOnlyTargetRules Target) : base(Target)
	{
	    bRequiresImplementModule = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"ApplicationCore",
				"Projects",
				"DataConfigCore",
				"DataConfigExtra",
				"DataConfigTests",
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"Launch",
			});
	}
}
