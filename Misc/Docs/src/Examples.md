# Examples

This page shows some short and quick examples showcasing DataConfig API usage and features. All code shown here can be found in DataConfig sources.

## JSON Deserialization

This is the example shown on front page. Given the `FDcTestExampleStruct`:

```c++
//	DataConfig/DataConfig/Source/DataConfigTests/Private/DcTestBlurb.h
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
//	DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
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

Note that `EDcTestExampleEnum` is deserialized by its name and `FColor` is deserialized from a html color string like `#RRGGBBAA`.

Say if we accidentally mistyped the `EnumField` value:

```json
{
    "StrField" : "Lorem ipsum dolor sit amet",
    "EnumField" : "Far",
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

## Custom Deserializer

To deserialize `FColor` with  `#RRGGBBAA` we'll need to provide custom logic to the deserializer. First is to implement a  `FDcDeserializePredicate` delegate to select `FColor` during deserialization:

```c++
//	DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeColor.cpp
EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == TBaseStructure<FColor>::Get()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}
```

Then we'll need to implement a `FDcDeserializeDelegate` to deserialize a `FColor`. We have a few options when doing it. Here we'll demonstrate how the `FDcWriter` API works:

```c++
//	DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeColor.cpp
template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriterAPI>(const FColor& Color, FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Writer->WriteStructRoot(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

	DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

	DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

	DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

	DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

	DC_TRY(Ctx.Writer->WriteStructEnd(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

	return DcOk();
}
```

Then we'll need to register these pair of delegates to the `FDcDeserializer`.

```c++
//	DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
FDcDeserializer Deserializer;
DcSetupJsonDeserializeHandlers(Deserializer);
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
    FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);
```

And then it's done. It would work recursively on `FColor` everywhere, like in `UCLASS/USTRUCT` members, in `TArray/TSet` and in `TMap` as key or values.

## Debug Dump

`DcAutomationUtils::DumpToLog()` can dump a Property + Data Pointer pair into a string representation.

