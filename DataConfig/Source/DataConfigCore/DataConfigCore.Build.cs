using UnrealBuildTool;

public class DataConfigCore : ModuleRules
{
	public DataConfigCore(ReadOnlyTargetRules Target) : base(Target)
	{
		//	source type module, pay extra attention to api boundry
	    bRequiresImplementModule = false;

	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnforceIWYU = true;

        PublicDependencyModuleNames.AddRange(
			new string[] {
			"Core",
			"CoreUObject",
			});

		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("DC_BUILD_FAST=1");
		}
		else
		{
			PublicDefinitions.Add("DC_BUILD_DEBUG=1");
		}

		//	toggle for debug unity
        //bUseUnity = false;
	}
}
