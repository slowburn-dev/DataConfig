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
#if UE_5_0_OR_LATER
				"DataConfigTests5",
#endif
#if UE_5_4_OR_LATER
				"DataConfigTests54",
#endif
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"Launch",
			});
	}
}
