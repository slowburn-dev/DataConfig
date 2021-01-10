using UnrealBuildTool;

public class DataConfigCore : ModuleRules
{
	public DataConfigCore(ReadOnlyTargetRules Target) : base(Target)
	{
		//	ref: AgogCore.Build.cs
		//	source type module
	    bRequiresImplementModule = false;
	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		//	TODO what this even does?
		bEnforceIWYU = true;

		PublicDependencyModuleNames.AddRange(
			new string[] {
			"Core",
			"CoreUObject",
            //  !!! can't referenec Engine, it neeeds `bCompileAgainstEngine` in UE4 Target
            //      but it failes to build as a console application

			//"Engine",	// need this for Blueprint.h
			});
	}
}
