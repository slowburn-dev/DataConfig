# Integration

## Use `DataConfigCore` Module

The most straight forward way is to directly use `Source/DataConfigCore` module. It only depends on `Core` and `CoreUObject` and itself is a source only module (ie doesn't do `IMPLEMENT_MODULE`). 

Say you have a game project setup at `C:/MyProject/MyProject.uproject`. We'll setup `DataConfig` as a editor only module for the game.

1. Copy `Source/DataConfigCore`, `Source/DataConfigTests`, `Source/DataConfigEditorExtra`, `Source/DataConfigTests` into `C:/MyProject/Source`.
  This step integrates most of DataConfig's source.
2. Edit `Source/MyProjectEditor.uproject` to add `DataConfigEditorExtra` as a ...

!!! note that must include `DataConfigEditorExtra` as it initialized DcEnv(), others just doesnt' do it at all.

