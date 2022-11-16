using UnrealBuildTool;

public class DcProjectTarget : TargetRules
{
    public DcProjectTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange( new string[] { "DcProjectGame" } );

#if UE_5_1_OR_LATER
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif
    }
}
