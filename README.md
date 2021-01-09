# DataConfig

__flexible and robust serialization framework for Unreal Property System__

## Features

- Support every type in Unreal Property System, including delegates and `UObject` references
- Pull/Push styled API for accurate type info, proper error handling and diagnostics.
- Out of the box JSON parsing that supports custom deserialization.
- Provide tools and framework for extending.

## Example

Give a struct annotated by Unreal Property System:

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

**DataConfig** can deserialize an instance from JSON with the code below:

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

//  prepare context for this run
FDcPropertyDatum Datum(FDcTestExampleStruct::StaticStruct(), &Dest);
FDcJsonReader Reader(Str);
FDcPropertyWriter Writer(Datum);

FDcDeserializeContext Ctx;
Ctx.Reader = &Reader;
Ctx.Writer = &Writer;
Ctx.Deserializer = &Deserializer;
Ctx.Properties.Push(Datum.Property);
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

Note that enum is deserialized by name and `FColor` is deserialized from a html color string like `#RRGGBBAA`.

Say if we accidentally mistyped the `EnumField` value:

```json
{
	"StrField" : "Lorem ipsum dolor sit amet",
	"EnumField" : "Far",    // <- i
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

## Documentation

See LINK_TO_MDBOOK_DOC for details.

## Credit Us

It would be trememdously helpful for us if you credit us in your game / projects. See LINK_TO_CREDIT for more details.
