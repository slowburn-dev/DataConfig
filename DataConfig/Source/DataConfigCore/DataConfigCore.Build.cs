using UnrealBuildTool;

public class DataConfigCore : ModuleRules
{
	public DataConfigCore(ReadOnlyTargetRules Target) : base(Target)
	{
		//	ref: AgogCore.Build.cs
		//	source type module
	    bRequiresImplementModule = false;
	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
			"Core",
			"CoreUObject",
			});
	}
}
