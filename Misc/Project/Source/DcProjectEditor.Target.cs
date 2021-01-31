using UnrealBuildTool;

public class DcProjectEditorTarget : TargetRules
{
    public DcProjectEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange( new string[] { "DcProjectGame", "DcProjectEditor", } );
    }
}
