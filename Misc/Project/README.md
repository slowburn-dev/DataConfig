**Sample Project**

UE4 needs to operate on a actual project for plugins and editor modules to work.

This serves as a minimal sample project that can be used to build DataConfig in a editor environment.


```
# build
C:\UnrealEngine\Engine\Binaries\DotNET\UnrealBuildTool DcProjectEditor Win64 Development "%CD%/DcProject.uproject" -Rocket -NoHotReload

# generate VS solution
C:\UnrealEngine\Engine\Binaries\DotNET\UnrealBuildTool DcProjectEditor Win64 Development "%CD%/DcProject.uproject" -Rocket -ProjectFiles -Game -Engine -NoIntellisense

# run editor
C:\UnrealEngine\Engine\Binaries\Win64\UE4Editor "%CD%/DcProject.uproject"
```