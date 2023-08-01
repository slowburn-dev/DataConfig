using UnrealBuildTool;

public class DcProjectEditor : ModuleRules
{
    public DcProjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.Full;
#else
        bEnforceIWYU = true;
#endif


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
