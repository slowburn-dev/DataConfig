using UnrealBuildTool;

public class DataConfigEditorExtra : ModuleRules
{
    public DataConfigEditorExtra(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core",
            "CoreUObject",
            "Engine",
            "MessageLog",
            "GameplayTags",
            "DataConfigCore",
            "DataConfigExtra",
            });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            });

    }
}
