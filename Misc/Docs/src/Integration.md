# Integration

## Integrate `DataConfig` Plugin

You should try adding DataConfig as a plugin into your C++ project to get started.  In this section we'll walk through the steps of integrating DataConfig plugin into a empty UE C++ Project.

1. Get a copy of this repository. Then copy  `./DataConfig` (where `DataConfig.uplugin` resides) into your project's `Plugins` directory.

2. Restart your project. There should be a prompt to compile plugin sources. Confirm and wait until your project launches. Then open `Settings -> Plugins` you should see **Data Config** listed under Project Editor category.

   ![Integration-DataConfigPlugin](Images/Integration-DataConfigPlugin.png)

3. The plugin comes with a set of tests. Open menu `Window -> Developer Tools  -> Session Frontend`. Find and run the `DataConfig` tests and it should all pass.
  
   ![Integration-DataConfigAutomations](Images/Integration-DataConfigAutomations.png)

## Integrate `DataConfigCore` Module

DataConfig is packed into a plugin to bundle in the automation tests with a few assets. You're encouraged to integrate only the `DataConfigCore` module. It contains all core features with minimal dependencies. 

Most projects should has a editor module already setup. In this section we'll go through the steps of integrating `DataConfigCore` and build it with the project's `FooProjectEditor` module.

1. Get a copy of this repository. Then copy `DataConfig/Source/DataConfigCore` into your project's `Source` directory.

2. Edit `FooProjectEditor.Build.cs` add add `DataConfigCore` as an extra module:

   ```c++
   using UnrealBuildTool;
   
   public class FooProjectEditor : ModuleRules
   {
       public FooProjectEditor(ReadOnlyTargetRules Target) : base(Target)
       {
           PublicDependencyModuleNames.AddRange(new string[] { 
   			//...
               "DataConfigCore",	// <- add this
               });
       }
   }
   ```

3. DataConfig needs to be explicitly initialized before use. Find `FooProjectEditor` module's start up and shut down methods and setup DataConfig accordingly.

   ```c++
   #include "DataConfig/DcEnv.h"
   #include "DataConfig/Automation/DcAutomationUtils.h"
   
   void FFooProjectEditorModule::StartupModule()
   {
       // ...
   	DcStartUp(EDcInitializeAction::SetAsConsole);
       // dump a FVector to try it out
   	k
   }
   
   void FFooProjectEditorModule::ShutdownModule()
   {
       // ...
   	DcShutDown();
   }
   ```
   
4. Rebuild the project and restart the editor. Open `Output Log` and use `LogDataConfigCore` filter and find the dump output. 

   ![Integration-DataConfigCoreOutput](Images/Integration-DataConfigCoreOutput.png)

You can refer to `DataConfigEditorExtra` module for more detailed integration options.