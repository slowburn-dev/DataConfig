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
			"Core",
			"CoreUObject",
			"PropertyPath",
			"Json",
			"JsonUtilities",
			"DataConfigCore",
			});

	}
}
