# Extra Module Setups

[Unreal Engine Modules][1] is how the engine handles its C++ code physical design. You'll need to be pretty familar with the system to scale up your C++ project.

We've split extra samples in two modules. First one is `DataConfigExtra` which does **not** depend on `Engine/UnrealEd` module. It can be built along program target. `DataConfigExtra` is basically a set of C++ source files bundled and there's no special setup for it. The key setup is to set `ModuleRules.bRequiresImplementModule` to be `false`:

```c#
// DataConfig/Source/DataConfigExtra/DataConfigExtra.Build.cs
public class DataConfigExtra : ModuleRules
{
	public DataConfigExtra(ReadOnlyTargetRules Target) : base(Target)
	{
	    bRequiresImplementModule = false;
	    Type = ModuleType.CPlusPlus;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(
			new string[] {
			"DataConfigCore",
			"Core",
			"CoreUObject",
             // ...
			});
	}
}
```

The other module is `DcEditorExtraModule` which is a more conventional editor only module. It's also the recommended setup for integrating DataConfig as a editor only dependency into your project. We also put samples that depends on `Engine` and other gameplay system in here.

Most of intergration code is in `IModuleInterface::StartUpModule/ShutdownModule()`. 

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DcEditorExtraModule.cpp
void FDcEditorExtraModule::StartupModule()
{
    // ...
	DcRegisterDiagnosticGroup(&DcDExtra::Details);
	DcRegisterDiagnosticGroup(&DcDEditorExtra::Details);

	DcStartUp(EDcInitializeAction::Minimal);
	DcEnv().DiagConsumer = MakeShareable(new FDcMessageLogDiagnosticConsumer());
	// ...
}

void FDcEditorExtraModule::ShutdownModule()
{
	DcShutDown();
    // ...
}

```

Here's a check list for integrating.

- Register additional diagnostics early. 
- Call `DcStartUp()/DcShutDonw()` pair.
- Register custom diagnostic consumer.

`FDcMessageLogDiagnosticConsumer` is an example of redirecting diagnostics into UE Message Log window with its own category.

[1]: https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/ProgrammingWithCPP/Modules/ "								Unreal Engine Modules 						"