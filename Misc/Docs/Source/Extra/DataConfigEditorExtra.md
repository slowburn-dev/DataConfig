# DataConfigEditorExtra

Module `DataConfigEditorExtra` contains examples that need to be run in a editor context.

## `DcEditorExtraModule`

This is a reference of integrating `DataConfigCore` in a editor module. A simple checklist:

- Register additional diagnostics early. 
- Call `DcStartUp()/DcShutDonw()` pair.
- Register custom diagnostic consumer.

`FDcMessageLogDiagnosticConsumer` is an example of redirecting diagnostics into UE Message Log window with its own category.

## Deserialize GameplayTags

`GameplayTags` is a built-in runtime module that implements hierarchical tags:

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

Note that gameplay tag parsing has error reporting builtin. In this case we can pipe it into the diagnostic easily:

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

## Deserialize Blueprint Class Instances

The Property System is powerful enough that you can create new Blueprint Class, which is equivalent to C++ `UCLASS` to some extents, in the editor. DataConfig can support these with some extra efforts.

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

There's a quirk that Blueprint class and fields names are actually mangled. The class







