# DataConfigEditorExtra

Module `DataConfigEditorExtra` contains examples that need to be run in a editor context.

## `DcEditorExtraModule`

This is a good reference of integrating `DataConfigCore` in a editor module. Here's a checklist:

- Register additional diagnostics early. 
- Call `DcStartUp()/DcShutDonw()` pair.
- Register custom diagnostic consumer.

`FDcMessageLogDiagnosticConsumer` is an example of redirecting diagnostics into UE Message Log window with its own category.


## Deserialize Blueprint Class Instances

The Property System is so powerful that you can create new Blueprint Class, which is equivalent to C++ `UCLASS` to some extents, within the Blueprint Editor. In this example we implemented deserializing these.

Blueprint Class can be referenced by the blueprint asset path:

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
FString Str = TEXT(R"(
    {
        //...
        "ClassField3" : "/DataConfig/DcFixture/DcTestBlueprintClassBeta",
    }
)");

UTEST_TRUE("...", Dest.ClassField3->GetFName() == TEXT("DcTestBlueprintClassBeta_C"));
```

And Blueprint structs can also be deserialized from JSON. We need to rewrite the handler `HandlerBPDcAnyStructDeserialize` for looking up Blueprint Struct by path:

```C++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
FString Str = TEXT(R"(
    {
        "AnyStructField1" : {
            "$type" : "/DataConfig/DcFixture/DcTestBlueprintStructWithColor",
            "NameField" : "Foo",
            "StrField" : "Bar",
            "IntField" : 123,
            "ColorField" : "#FF0000FF"
        }
    }
)");
```

There's a quirk that Blueprint Struct actually mangle its field names. The struct above dumps to something like this:

```
-----------------------------------------
# Datum: 'UserDefinedStruct', 'DcTestBlueprintStructWithColor'
<StructRoot> 'DcTestBlueprintStructWithColor'
|---<Name> 'NameField_5_97BFF114405C1934C2F33E8668BF1652'
|---<Name> 'Foo'
|---<Name> 'StrField_9_FAA71EFE4896F4E6B1478B9C13B2CE52'
|---<String> 'Bar'
|---<Name> 'IntField_11_3BC7CB0F42439CE2196F7AA82A1AC374'
|---<Int32> '123'
|---<Name> 'ColorField_14_F676BCF245B2977B678B65A8216E94EB'
|---<StructRoot> 'Color'
|   |---<Name> 'B'
|   |---<UInt8> '0'
|   |---<Name> 'G'
|   |---<UInt8> '0'
|   |---<Name> 'R'
|   |---<UInt8> '255'
|   |---<Name> 'A'
|   |---<UInt8> '255'
|---<StructEnd> 'Color'
<StructEnd> 'DcTestBlueprintStructWithColor'
-----------------------------------------
```

The good news is that DataConfig already got this covered. 

## Deserialize GameplayTags

[GameplayTags][1] is a built-in runtime module that implements hierarchical tags. In this example we implemented deserializing into `FGameplayTag` from a string.

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.cpp
FString Str = TEXT(R"(
    {
        "TagField1" : null,
        "TagField2" : "DataConfig.Foo.Bar"
    }
)");

UTEST_FALSE("...", Dest.TagField1.IsValid());
UTEST_TRUE("...", Dest.TagField2.IsValid());
UTEST_TRUE("...", Dest.TagField2 == UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")));
```

We also implemented deserializing `FGameplayTagContainer` from a list of strings:

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.cpp
FString Str = TEXT(R"(
    {
        "TagContainerField1" : [],
        "TagContainerField2" : [
            "DataConfig.Foo.Bar",
            "DataConfig.Foo.Bar.Baz",
            "DataConfig.Tar.Taz",
        ]
    }
)");

UTEST_TRUE("...", Dest.TagContainerField1.Num() == 0);
UTEST_TRUE("...", Dest.TagContainerField2.Num() == 3);
UTEST_TRUE("...", Dest.TagContainerField2.HasTagExact(
    UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar"))
));
UTEST_TRUE("...", Dest.TagContainerField2.HasTagExact(
    UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"))
));
UTEST_TRUE("...", Dest.TagContainerField2.HasTagExact(
    UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Tar.Taz"))
));

```

Note that gameplay tag parsing has error reporting built-in. In this case we can pipe it into our diagnostic:

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.cpp
static FDcResult _StringToGameplayTag(FDcDeserializeContext& Ctx, const FString& Str, FGameplayTag* OutTagPtr)
{
    FString FixedString;
    FText Err;
    if (!FGameplayTag::IsValidGameplayTagString(Str, &Err, &FixedString))
    {
        return DC_FAIL(DcDEditorExtra, InvalidGameplayTagStringFixErr)
            << Str << FixedString << Err;
    }
    //...
}
```

In case of a invalid tag it would report the reason and fixed string:

```
# DataConfig Error: Invalid Gameplay Tag String, Actual: 'DataConfig.Invalid.Tag.', Fixed: 'DataConfig.Invalid.Tag', Error: 'Tag ends with .'
- [JsonReader] --> <in-memory>5:4
   3 |        { 
   4 |            "TagField1" : null, 
   5 |            "TagField2" : "DataConfig.Invalid.Tag." 
     |                          ^^^^^^^^^^^^^^^^^^^^^^^^^
   6 |        } 
   7 |    
- [PropertyWriter] Writing property: (FDcEditorExtraTestStructWithGameplayTag1)$root.(FGameplayTag)TagField2
 [C:\DevUE\UnrealEngine\Engine\Source\Developer\MessageLog\Private\Model\MessageLogListingModel.cpp(73)]
```

## Deserialize Gameplay Abilities

We'll conclude examples with a concrete user story: populating `GameplayAbility` and `GameplayEffect` blueprint from JSON file.

[Gameplay Ability System][2] is a built-in plugin for building data driven abilities. Users are expected to derived and modify `GameplayAbility` and `GameplayEffect` blueprint for custom logic. 

Given a JSON like this:

```c++
// DataConfig/Tests/Fixture_AbilityAlpha.json
{
    /// Tags
    "AbilityTags" : [
        "DataConfig.Foo.Bar",
        "DataConfig.Foo.Bar.Baz",
    ],
    "CancelAbilitiesWithTag" : [
        "DataConfig.Foo.Bar.Baz",
        "DataConfig.Tar.Taz",
    ],
    /// Costs
    "CostGameplayEffectClass" : "/DataConfig/DcFixture/DcTestGameplayEffectAlpha",
    /// Advanced
    "ReplicationPolicy" : "ReplicateYes",
    "InstancingPolicy" : "NonInstanced",
}
```

Right click on a `GameplayAbility` blueprint asset and select `Load From JSON`. Select this file and it would correctly populate the fields with the values above, as seen in the pic below:

![DataConfigEditorExtra-LoadJsonIntoAbility](Images/DataConfigEditorExtra-LoadJsonIntoAbility.png)

Most of the logic is in `DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.cpp`

- The context menu is added from `GameplayAbilityEffectExtender`. There's another handy item named `Dump To Log` which dumps any blueprint CDO into the log.
- DataConfig deserializer is setup in `LazyInitializeDeserializer()`. We added custom logic for deserializing `FGameplayAttribute` from a string like `DcTestAttributeSet.Mana`.
- We also reused many methods from previous examples to support `FGameplayTag` deserialization and Blueprint class look up by path.

[1]: https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Tags/index.html "Gameplay Tags"
[2]: https://docs.unrealengine.com/en-US/InteractiveExperiences/GameplayAbilitySystem/index.html "Gameplay Ability System"

