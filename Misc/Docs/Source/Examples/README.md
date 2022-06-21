# Examples

Here're some short and quick examples showcasing DataConfig API usage and features. All code shown here can be found in the [repo][1].

## JSON Deserialization

* [DcTestBlurb.cpp]({{SrcRoot}}DataConfigTests/Private/DcTestBlurb.cpp#L20)

Given the struct`FDcTestExampleStruct`:

```c++
// DataConfigTests/Private/DcTestBlurb.h
UENUM()
enum class EDcTestExampleEnum
{
    Foo, Bar, Baz
};

USTRUCT()
struct FDcTestExampleStruct
{
    GENERATED_BODY()
    UPROPERTY() FString StrField;
    UPROPERTY() EDcTestExampleEnum EnumField;
    UPROPERTY() TArray<FColor> Colors;
};
```

We can deserialize an instance from JSON with the snippet below:

```c++
// DataConfigTests/Private/DcTestBlurb.cpp
FString Str = TEXT(R"(
    {
        "StrField" : "Lorem ipsum dolor sit amet",
        "EnumField" : "Bar",
        "Colors" : [
            "#FF0000FF", "#00FF00FF", "#0000FFFF"
        ]
    }
)");

FDcTestExampleStruct Dest;

//  create and setup a deserializer
FDcDeserializer Deserializer;
DcSetupJsonDeserializeHandlers(Deserializer);
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
    FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);

//  prepare context for this run
FDcPropertyDatum Datum(&Dest);
FDcJsonReader Reader(Str);
FDcPropertyWriter Writer(Datum);

FDcDeserializeContext Ctx;
Ctx.Reader = &Reader;
Ctx.Writer = &Writer;
Ctx.Deserializer = &Deserializer;
DC_TRY(Ctx.Prepare());

//  kick off deserialization
DC_TRY(Deserializer.Deserialize(Ctx));

//  validate results
check(Dest.StrField == TEXT("Lorem ipsum dolor sit amet"));
check(Dest.EnumField == EDcTestExampleEnum::Bar);
check(Dest.Colors[0] == FColor::Red);
check(Dest.Colors[1] == FColor::Green);
check(Dest.Colors[2] == FColor::Blue);
```

Note that `EDcTestExampleEnum` is deserialized by its name and `FColor` is deserialized from a html color string like `#RRGGBBAA`.

Say if we accidentally mistyped the `EnumField` value:

```json
{
    "StrField" : "Lorem ipsum dolor sit amet",
    "EnumField" : "Far",
}
```

It would fail gracefully with diagnostics:

```
# DataConfig Error: Enum name not found in enum type: EDcTestExampleEnum, Actual: 'Far'
- [JsonReader] --> <in-memory>4:25
   2 |    {
   3 |        "StrField" : "Lorem ipsum dolor sit amet",
   4 |        "EnumField" : "Far",
     |                           ^
   5 |        "Colors" : [
   6 |            "#FF0000FF", "#00FF00FF", "#0000FFFF"
- [PropertyWriter] Writing property: (FDcTestExampleStruct)$root.(EEDcTestExampleEnum)EnumField
```

As bonus we serialize the struct into MsgPack: 

```c++
FDcSerializer Serializer;
DcSetupMsgPackSerializeHandlers(Serializer);

FDcPropertyDatum Datum(&Dest);
FDcPropertyReader Reader(Datum);
FDcMsgPackWriter Writer;

//  prepare serialize context
FDcSerializeContext Ctx;
Ctx.Reader = &Reader;
Ctx.Writer = &Writer;
Ctx.Serializer = &Serializer;
DC_TRY(Ctx.Prepare());

//  kick off serialization
DC_TRY(Serializer.Serialize(Ctx));

auto& Buffer = Writer.GetMainBuffer();
//  starts withMsgPack FIXMAP(3) header
check(Buffer[0] == 0x83);   
```

[1]: https://github.com/slowburn-dev/DataConfig/blob/release/DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp "DcTestBlurb.cpp"