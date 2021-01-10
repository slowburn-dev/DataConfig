using UnrealBuildTool;

public class DataConfigCore : ModuleRules
{
	public DataConfigCore(ReadOnlyTargetRules Target) : base(Target)
	{
		//	source type module, pay extra attention to api boundry
	    bRequiresImplementModule = false;

	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnforceIWYU = true;

		PublicDependencyModuleNames.AddRange(
			new string[] {
			"Core",
			"CoreUObject",
			});
	}
}
