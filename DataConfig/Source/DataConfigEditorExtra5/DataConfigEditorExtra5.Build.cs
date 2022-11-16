using UnrealBuildTool;

public class DataConfigEditorExtra5 : ModuleRules
{
    public DataConfigEditorExtra5(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "DataConfigCore",
            "DataConfigExtra",
            "StructUtils",
            });
    }
}
