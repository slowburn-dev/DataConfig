using UnrealBuildTool;

public class DataConfigRuntime : ModuleRules
{
	public DataConfigRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		// PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				// "DataConfigRuntime/Private",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
			});

/*
#if WITH_FORWARDED_MODULE_RULES_CTOR
        if (Target.bBuildEditor)
#else
        if (UEBuildConfiguration.bBuildEditor)
#endif
        {
        	//	try get faster iteration
        	OptimizeCode = CodeOptimization.Never;
        	bFasterWithoutUnity = true;
        	//	pch ONLY when iterating as it doesn't make sense in release anyway
			PrivatePCHHeaderFile = "Private/NGLuaConfigRuntimePCH.h";
        }
*/

	}
}
