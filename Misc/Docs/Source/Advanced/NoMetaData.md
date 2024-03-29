# No MetaData

Some DataConfig behaviors take advantage of [property metadata][1]. For example the default `DcSkip` metadata let DataConfig ignore marked property:

```c++
//  DataConfigTests/Private/DcTestProperty3.h
UPROPERTY(meta = (DcSkip)) int SkipField1;
```

However the metadata would be stripped in packaged builds and `FProperty::GetMetaData/HasMetaData()` set of methods would be gone. The state is roughly like this:

1. In [packaged builds][2] there's no metadata C++ methods nor any data.
2. Program targets can choose whether to keep metadata methods by setting `bBuildWithEditorOnlyData` in `Target.cs`. This toggles macro `WITH_EDITORONLY_DATA`.
   However the actual metadata would *not* be auto loaded.
3. In editor metadata works as intended.

In release 1.4.1 we fixed compilation on `!WITH_EDITORONLY_DATA`. This would allow you to include `DataConfigCore` as a runtime module and ship it with your project. This is an intended and supported use case.

However with no metadata some default DataConfig behaviors would be different:

* All builtin metas (`DcSkip/DcMsgPackBlob`) are ignored.
* Bitflag enums (`UENUM(meta = (Bitflags))`) are ignored. All enums are now treated as scalar.

The good news is that you can workaround this [by writing custom serialization handlers](../Programming/SerializerDeserializer.md). For example for bitflag enums:

* Name your bitflag enums with a prefix like `EFlagBar` and check for it in predicates.
* Collect a list of bitflag enum names and test for it in predicates.
* On serializing `PeekRead` for next data entry. If it's a `ArrayRoot` then treat it as a bitflag enum.

Note that you should be able to implement these without modifying `DataConfigCore` code.

[1]:https://docs.unrealengine.com/en-US/metadata-specifiers-in-unreal-engine/
[2]:https://docs.unrealengine.com/en-US/packaging-unreal-engine-projects/
