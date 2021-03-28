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
            "UnrealEd",
            "MessageLog",
            "DesktopPlatform",
            "GameplayTags",
            "GameplayAbilities",
            "DataConfigCore",
            "DataConfigExtra",
            });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "ToolMenus",
            "Kismet",
            "KismetCompiler",
            });

    }
}
