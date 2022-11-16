# DataConfig

__Serialization framework for Unreal Engine that just works!__

Unreal Engine features a powerful [Property System][1] which implements C++ runtime reflection. **DataConfig** is a serialization framework build on top of it. Notably features:

- Out of the box JSON/MsgPack read write.
- Full support for UPROPERTY()/UCLASS()/USTRUCT()/UENUM().
- Pull/Push styled API for verbatim data access and lossless type infomation.
- Designed as a collection of tools that can be easily extended to support other formats.

## Documentation

See [DataConfig Book][2] for details.

## Example

Given a struct annotated and processed with Unreal's Property System:

```c++
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

**DataConfig** can deserialize an instance from JSON with the snippet below:

```c++
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

//  prepare deserialize context
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

Note that enum is deserialized by its name and `FColor` is deserialized from a html color string like `#RRGGBBAA`.

Say if we accidentally mistyped the `EnumField` value:

```json
{
    "StrField" : "Lorem ipsum dolor sit amet",
    "EnumField" : "Far",
```

**DataConfig** would fail gracefully with diagnostics:

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
## License

DataConfig is released under MIT License.

It would be trememdously helpful if you credit us in your projects. See the [license page][3] for details.

[1]:https://www.unrealengine.com/en-US/blog/unreal-property-system-reflection "Unreal Property System (Reflection)"
[2]:https://slowburn.dev/dataconfig "DataConfig Book"
[3]:https://slowburn.dev/dataconfig/License "DataConfig Book - License"
