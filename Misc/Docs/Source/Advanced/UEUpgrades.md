# Unreal Engine Upgrades

DataConfig is committed to support multiple UE versions with no deprecations and warnings. On this page we'll document important upgrade and migration info.

# UE5.3

* Introduces `BuildSettingsVersion.V4` which now defaults to C++ 20.
* `TRemoveConst` is deprecated over `std::remove_const`.
* `FScriptDelegate` etc now has additional checkers based on threading model and debug/release build. Thus we change how `FScriptDelegateAccess` works.

# UE5.2

* `TIsSame` is deprecated over `std::is_same`.
* In `Build.cs` `bEnforceIWYU` is changed to enum `IWYUSupport`.

# UE5.1

* UE5.1 deprecates `ANY_PACKAGE` in favor of a new method `FindFirstObject`. In DataConfig we provided `DcSerdeUtils::FindFirstObject` which calls `FindObject(ANY_PACKAGE)` pre 5.1 and calls `FindFirstObject()` for 5.1 and onwards. 

# UE5.0

* New [`TObjectPtr`][2] to replace raw UObject pointers. Turns out this is mostly handled within the engine and is transparent to DataConfig.

* New property types `FObjectPtrProperty` and `FClassPtrProperty` are added. They're handled the same as `FObjectProperty` and `FClassProperty` respectively.

* `FVector` now is 3 `double`s, and `Real` data type in Blueprint now is also double. This is also mostly transparent to DataConfig.

* `FScriptArrayHelperAccess` size changes with a added `uint32 ElementAlignment`.

* `TStringBuilderWithBuffer` API changes. At call sites we now do `Sb << TCHAR('\n')` instead of `Sb.Append(TCHAR('\n'))` .

# UE4

* The oldest version DataConfig supports is UE 4.25, in which it [introduces a major refactor][1] that changes `UProperty` to `FProperty`. We intended to support UE4 in the foreseeable future, especially when we now have separated `uplugin` for UE4 and UE5.


[1]:https://docs.unrealengine.com/4.27/en-US/WhatsNew/Builds/ReleaseNotes/4_25/#new:uobjectpropertyoptimizations
[2]:https://docs.unrealengine.com/5.0/en-US/unreal-engine-5-migration-guide/#c++objectpointerproperties