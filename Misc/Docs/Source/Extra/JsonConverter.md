# JsonConverter in DataConfig

UE comes with a handy module [`JsonUtilities`][1] that handles conversion between `USTRUCT`s and JSON. In this example we've implemented similar functionalities that behaves almost identical to stock `FJsonConverter`.

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcJsonConverter.cpp
FString Str = TEXT(R"(
    {
        "strField" : "Foo",
        "nestField" : {
            "strArrayField" : [
                "One",
                "Two",
                "Three"
            ],
            "strIntMapField" : {
                "One": 1,
                "Two": 2,
                "Three": 3
            }
        },
        "intField" : 253,
        "boolField" : true
    }
)");

{
	FDcTestJsonConverter1 Lhs;
	bool LhsOk = DcExtra::JsonObjectStringToUStruct(Str, &Lhs);

    FDcTestJsonConverter1 Rhs;
	bool RhsOk = FJsonObjectConverter::JsonObjectStringToUStruct(Str, &Rhs);
}    
{
	FString Lhs;
	bool LhsOk = DcExtra::UStructToJsonObjectString(Data, Lhs);

	FString Rhs;
	bool RhsOk = FJsonObjectConverter::UStructToJsonObjectString(Data, Rhs);
}
```

`DcExtra::JsonObjectStringToUStruct()` body is trivia as it delegates most of the work to `DcDeserializer`:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcJsonConverter.cpp
bool JsonObjectReaderToUStruct(FDcReader* Reader, FDcPropertyDatum Datum)
{
	FDcResult Ret = [&]() -> FDcResult {
		using namespace JsonConverterDetails;
		LazyInitializeDeserializer();

		FDcPropertyWriter Writer(Datum);

		FDcDeserializeContext Ctx;
		Ctx.Reader = Reader;
		Ctx.Writer = &Writer;
		Ctx.Deserializer = &Deserializer.GetValue();
		DC_TRY(Ctx.Prepare());

		DC_TRY(Deserializer->Deserialize(Ctx));
		return DcOk();
	}();

	if (!Ret.Ok())
	{
		DcEnv().FlushDiags();
		return false;
	}
	else
	{
		return true;
	}
}
```

The serializing function `DcExtra::UStructToJsonObjectString()` needs some customization as default `DcJsonWriter` and `DcSerializer` handlers behaves a bit different against stock `FDcJsonConverter`:

* It serialize field names as `camelCase`.
* It uses platform dependent line endings, that is `\r\n` on Windows.
* It have sutble new line breaking rules on nested array and object, and on spacing around `:` token.

The good news is that one can customize these behaviors with DataConfig to match it:

```c++
// DataConfig/Source/DataConfigExtra/Public/DataConfig/Extra/Types/DcJsonConverter.h
template<typename InStructType>
static bool UStructToJsonObjectString(const InStructType& InStruct, FString& OutJsonString)
{
    static FDcJsonWriter::ConfigType _JSON_CONVERTER_CONFIG = []
	{
		FDcJsonWriter::ConfigType Config = FDcJsonWriter::DefaultConfig;
		Config.IndentLiteral = TEXT("\t");
		Config.LineEndLiteral = LINE_TERMINATOR;
		Config.LeftSpacingLiteral = TEXT("");
		Config.bNestedArrayStartsOnNewLine = false;
		Config.bNestedObjectStartsOnNewLine = true;
		return Config;
	}();

	FDcJsonWriter Writer(_JSON_CONVERTER_CONFIG);
    ...
}

// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcJsonConverter.cpp
static FDcResult HandlerStructRootSerializeCamelCase(FDcSerializeContext& Ctx)
{
    ...
	else if (CurPeek == EDcDataEntry::Name)
	{
		FName Value;
		DC_TRY(Ctx.Reader->ReadName(&Value));
		DC_TRY(Ctx.Writer->WriteString(FJsonObjectConverter::StandardizeCase(Value.ToString())));
    }
}
```

We aim to support flexible serialization and formatting behaviors without modifying `DataConfigCore` code:

[1]: https://docs.unrealengine.com/4.27/en-US/API/Runtime/JsonUtilities/