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

//	create and setup a deserializer
FDcDeserializer Deserializer;
DcSetupJsonDeserializeHandlers(Deserializer);
Deserializer.AddPredicatedHandler(
	FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
	FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);

//	prepare context for this run
FDcPropertyDatum Datum(FDcTestExampleStruct::StaticStruct(), &Dest);
FDcJsonReader Reader(Str);
FDcPropertyWriter Writer(Datum);

FDcDeserializeContext Ctx;
Ctx.Reader = &Reader;
Ctx.Writer = &Writer;
Ctx.Deserializer = &Deserializer;
Ctx.Properties.Push(Datum.Property);
DC_TRY(Ctx.Prepare());

//	kick off deserialization
DC_TRY(Deserializer.Deserialize(Ctx));

//	validate results
check(Dest.StrField == TEXT("Lorem ipsum dolor sit amet"));
check(Dest.EnumField == EDcTestExampleEnum::Bar);
check(Dest.Colors[0] == FColor::Red);
check(Dest.Colors[1] == FColor::Green);
check(Dest.Colors[2] == FColor::Blue);
```

Note that enum is deserialized by name and `FColor` is deserialized from a html color string like `#RRGGBBAA`.

Say if we accidentally forget to close the string in JSON:

```json
{
	"StrField" : "Lorem ipsum dolor sit amet, // <-- missing '"'
	"EnumField" : "Bar",
```

**DataConfig** would fail gracefully with diagnostics:

```
# DataConfig Error: Unclosed string literal
- [JsonReader] --> <in-memory>3:45
   2 |            {
   3 |                "StrField" : "Lorem ipsum dolor sit amet
     |                             ^
   4 |                "EnumField" : "Bar",
   5 |                "Colors" : [
- [PropertyWriter] Writing property: (FDcTestExampleStruct)$root.(FString)StrField
```

## Documentation

See LINK_TO_MDBOOK_DOC for details.

## Credit Us

It would be trememdously helpful for us if you credit us in your game / projects. See LINK_TO_CREDIT for more details.
