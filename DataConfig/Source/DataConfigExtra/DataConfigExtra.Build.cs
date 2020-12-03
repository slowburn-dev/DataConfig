using UnrealBuildTool;

public class DataConfigExtra : ModuleRules
{
	public DataConfigExtra(ReadOnlyTargetRules Target) : base(Target)
	{
	    bRequiresImplementModule = false;
	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
			//	still need these 2 for link to pass, wtf?
			"Core",
			"CoreUObject",

			"DataConfigCore",
			});

	}
}
