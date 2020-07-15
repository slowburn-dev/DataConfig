using UnrealBuildTool;

public class DataConfigEditor : ModuleRules
{
	public DataConfigEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				// "DataConfigEditor/Private",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"EditorStyle",
				"Engine",
				"UnrealEd",
				"DataConfigCore",
				"EngineSettings",	// for auto complete
				"MessageLog",		// for MessageLog window category
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
			PrivatePCHHeaderFile = "Private/DataConfigEditorPCH.h";
        }
        */
	}
}
