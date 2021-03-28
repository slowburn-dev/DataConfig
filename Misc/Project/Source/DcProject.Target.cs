using UnrealBuildTool;

public class DcProjectTarget : TargetRules
{
    public DcProjectTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange( new string[] { "DcProjectGame" } );
    }
}
