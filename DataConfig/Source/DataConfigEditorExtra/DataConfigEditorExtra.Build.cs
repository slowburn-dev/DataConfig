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
            "UnrealEd",
            "MessageLog",
            "PythonScriptPlugin",
            "DataConfigCore",
            });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            });
    }
}
