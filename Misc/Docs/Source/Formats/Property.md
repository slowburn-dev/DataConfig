# Property

The property system is at the heart of DataConfig: serializer read from property system, while deserializer writes into it. This is directly reflected in the types:

```c++
// DataConfigCore/Public/DataConfig/Serialize/DcSerializeTypes.h
struct DATACONFIGCORE_API FDcSerializeContext
{
    // ...
    FDcPropertyReader* Reader = nullptr;
    FDcWriter* Writer = nullptr;
};

// DataConfigCore/Public/DataConfig/Deserialize/DcDeserializeTypes.h
struct DATACONFIGCORE_API FDcDeserializeContext
{
    FDcReader* Reader = nullptr;
    FDcPropertyWriter* Writer = nullptr;
};
```

## Property Datum and Reader/Writer

* [DcPropertyDatum.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Property/DcPropertyDatum.h)
* [DcPropertyReader.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Property/DcPropertyReader.h)
* [DcPropertyWriter.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Property/DcPropertyWriter.h)

Property reader/writer are usually constructed from a `FDcPropertyDatum`, which is a "fat pointer" that represents an entry in the property system.

```c++
// DataConfigCore/Public/DataConfig/Property/DcPropertyDatum.h
struct DATACONFIGCORE_API FDcPropertyDatum
{
    FFieldVariant Property;
    void* DataPtr;
    //...
}
```

Recall that [DataConfig data model](../Programming/DataModel.md) expands to a set of `Read/Write` calls. In the example below we setup a `FDcPropertyReader` with a simple struct and dump it out:

```c++
//  DataConfigTests/Private/DcTestBlurb2.h
USTRUCT()
struct FDcTestExample2
{
    GENERATED_BODY()

    UPROPERTY() FString StrField;
    UPROPERTY() FString StrArrField[3];

    UPROPERTY() UDcBaseShape* InlineField;
    UPROPERTY() UDcTestClass1* RefField;
    // ...
};

//  DataConfigTests/Private/DcTestBlurb2.cpp
FDcTestExample2 Value;
Value.MakeFixture();

FDcPropertyDatum Datum(&Value);
FDcPropertyReader Reader(Datum);
DcAutomationUtils::DumpToLog(&Reader);
```

The results are like this:

```
<StructRoot> 'DcTestExample2'
|---<Name> 'StrField'
|---<String> 'Foo'
|---<Name> 'StrArrField'
|---<ArrayRoot>
|   |---<String> 'One'
|   |---<String> 'Two'
|   |---<String> 'Three'
|---<ArrayEnd>
|---<Name> 'InlineField'
|---<ClassRoot> 'DcShapeSquare'
|   |---<Name> 'Radius'
|   |---<Float> '2.000000'
|   |---<Name> 'ShapeName'
|   |---<Name> 'MyBox'
|---<ClassEnd> 'DcBaseShape'
|---<Name> 'RefField'
|---<ClassRoot> 'DcTestClass1'
|   |---<ObjectReference> '337' 'DcTestClass1_0'
|---<ClassEnd> 'DcTestClass1'
<StructEnd> 'DcTestExample2'
```

Some caveats:

* Note that `DcTestExample2::StrArrField` is dumped into a array. Note that DataConfig can't distinguish between `Foo` and `Foo[1]` that's a 1 element array. It would be read as a normal field.

* Note that `DcTestExample2::InlineField` is expanded and `DcTestExample2::RefField` is read as an reference. This is determined by `FDcPropertyConfig::ExpandObjectPredicate`, which by default expands:
    * Field marked with `UPROPERTY(Instanced)`
    * Class marked with `UCLASS(DefaultToInstanced, EditInlineNew)`

### Configuring with `FDcPropertyConfig`

* [DcPropertyTypes.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Property/DcPropertyTypes.h)

Property reader/writer accepts `FDcPropertyConfig` class for customizing behaviors. By default we've implemented `DcSkip` metadata that you can mark on `given reader/writer.URPOPERTY()` and the field would be skipped by given reader/writer:

```c++
//  DataConfigTests/Private/DcTestProperty3.h
USTRUCT()
struct FDcTestMeta1
{
    GENERATED_BODY()

    UPROPERTY(meta = (DcSkip)) int SkipField1;
};
```

Though that `DcSkip` behavior is enabled by default, you can override this with a custom `FPropertyConfig` instance. Here's an example of processing only fields with `DcTestSerialize` meta:

```c++
//  DataConfigTests/Private/DcTestProperty4.h
USTRUCT()
struct FDcTestSerializeMeta1
{
	GENERATED_BODY()

	UPROPERTY(meta = (DcTestSerialize)) int SerializedField;
	UPROPERTY() int IgnoredField;
};

//  DataConfigTests/Private/DcTestProperty4.cpp
FDcPropertyConfig Config;
//  only process fields that has `DcTestSerialize` meta
Config.ProcessPropertyPredicate = FDcProcessPropertyPredicateDelegate::CreateLambda([](FProperty* Property)
{
    const static FName TestSerializeMeta = FName(TEXT("DcTestSerialize"));
    return DcPropertyUtils::IsEffectiveProperty(Property)
        && Property->HasMetaData(TestSerializeMeta);
});
Config.ExpandObjectPredicate = FDcExpandObjectPredicateDelegate::CreateStatic(DcPropertyUtils::IsSubObjectProperty);
Ctx.Reader->SetConfig(Config);
```



## Pipe Property Handlers

* [DcPropertyPipeSerializers.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Serialize/Handlers/Property/DcPropertyPipeSerializers.h)
* [DcPropertyPipeSerializers.cpp]({{SrcRoot}}DataConfigCore/Private/DataConfig/Serialize/Handlers/Property/DcPropertyPipeSerializers.cpp)
* [DcPropertyPipeDeserializers.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.h)
* [DcPropertyPipeDeserializers.cpp]({{SrcRoot}}DataConfigCore/Private/DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.cpp)

There's a set of deserialize handlers in `DcPropertyPipeHandlers` namespace. It's used to roundtripping property system objects.

Simply speaking it's equivalent to doing a `FDcPipeVisitor`  pipe visit.

```c++
//  DataConfigTests/Private/DcTestBlurb.cpp
//  these two blocks are equivalent
{
    FDcPropertyReader Reader(FromDatum);
    FDcPropertyWriter Writer(ToDatum);
    FDcPipeVisitor RoundtripVisit(&Reader, &Writer);

    DC_TRY(RoundtripVisit.PipeVisit());
}

{
    FDcDeserializer Deserializer;
    DcSetupPropertyPipeDeserializeHandlers(Deserializer);

    FDcPropertyReader Reader(FromDatum);
    FDcPropertyWriter Writer(ToDatum);

    FDcDeserializeContext Ctx;
    Ctx.Reader = &Reader;
    Ctx.Writer = &Writer;
    Ctx.Deserializer = &Deserializer;
    DC_TRY(Ctx.Prepare());

    DC_TRY(Deserializer.Deserialize(Ctx));
}
```

These are provided as a set of basis to for building custom property wrangling utils. See [Field Renamer](../Extra/FieldRenamer.md) for example.