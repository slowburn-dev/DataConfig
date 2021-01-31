using UnrealBuildTool;

public class DcProjectEditor : ModuleRules
{
    public DcProjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core",
            "CoreUObject",
            "Engine"
            });
        PrivateDependencyModuleNames.AddRange(new string[] {
            "DcProjectGame",
            });
    }
}
