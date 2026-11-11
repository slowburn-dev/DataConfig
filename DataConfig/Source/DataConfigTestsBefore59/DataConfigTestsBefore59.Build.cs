using UnrealBuildTool;

public class DataConfigTestsBefore59 : ModuleRules
{
	public DataConfigTestsBefore59(ReadOnlyTargetRules Target) : base(Target)
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
