# Built-in Reader and Writers

This page briefly walks through `FDcReader/FDcWriter` classes bundled in `DataConfigCore`.

## `FDcPipeVisitor` and `FDcPrettyPrintWriter`

`FDcPipeVisitor` takes a `FDcReader` and a `FDcWriter` then start peek-read-write loop until it peeks `EDcDataEntry::Ended` from reader or an error happens.

Then there's `FDcPrettyPrintWriter` that dumps everything write into it as string.

Combining these two we get a way to dump arbitrary `FDcReader` into a string!. This is how built-in debug dump features are implemented:

```c++
// DataConfig/Source/DataConfigCore/Private/DataConfig/Automation/DcAutomationUtils.cpp
void DumpToOutputDevice(...)
{
    //...
    FDcPropertyReader PropReader(Datum);
    FDcPrettyPrintWriter PrettyWriter(Output);
    FDcPipeVisitor PrettyPrintVisit(&PropReader, &PrettyWriter);

    if (!PrettyPrintVisit.PipeVisit().Ok())
        ScopedEnv.Get().FlushDiags();
    //...
}
```
In following sections we'll see some other usages of the pipe visitor.

## `FDcPropertyReader/FDcPropertyWriter`

This pair of classes is used to access the actual Unreal Property System. Both takes a `FDcPropertyDatum` to construct:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Property/DcPropertyDatum.h
struct DATACONFIGCORE_API FDcPropertyDatum
{
    FFieldVariant Property;
    void* DataPtr;
    //...
}
```

It's simply a property plus opaque pointer pair. These constructs are also called "Fat Pointers". Turns out this is enough to represent everything in the Property System. Property reader/writer needs one of these as a root and start reading/writing from there.

A simple use case of these is to roundtrip two objects so that every property in the first one is copied into latter:

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestCommon.h
FDcResult DcTestPropertyRoundtrip(...)
{
    FDcPropertyReader Reader(FromDatum);
    FDcPropertyWriter Writer(ToDatum);
    //...
    FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
    return RoundtripVisit.PipeVisit();
}
```
There's a quirk that you cannot create a `FDcPropertyDatum` for stack allocated `TArray` :

```c++
void f()
{
    // this is ok
    FDcTestExampleStruct MyStruct;
    FDcPropertyDatum StructDatum(&FDcTestExampleStruct::StaticStruct(), &MyStruct);

    // this is not
    TArray<int> myArr;
    FDcPropertyDatum myDatum(&???, &myArr);
}
```

We can, however, get datum for member `TArray` and other fields. We'll see related example in later chapters.

## `FDcJsonReader`

This is the only reader that reads a external textual format. It's also an example showcasing that the DataConfig data model is actually a superset of the property system.

```c++
// DataConfig/DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
FString Str = TEXT(R"(
    {
        "Str":    "Fooo",
        "Number": 1.875,
        "Bool":   true
    } 
)");

FDcJsonReader Reader(Str);

//  calling read methods
FString KeyStr;
FString GotStr;
double GotNumber;
bool GotBool;

DC_TRY(Reader.ReadMapRoot());

    DC_TRY(Reader.ReadString(&KeyStr));
    DC_TRY(Reader.ReadString(&GotStr));

    DC_TRY(Reader.ReadString(&KeyStr));
    DC_TRY(Reader.ReadDouble(&GotNumber));

    DC_TRY(Reader.ReadString(&KeyStr));
    DC_TRY(Reader.ReadBool(&GotBool));

DC_TRY(Reader.ReadMapEnd());

//  validate results
check(GotStr == TEXT("Fooo"));
check(GotNumber == 1.875);
check(GotBool == true);
```

In the example above we deserialized a `JSON` object from string. The first and the last call is `ReadMapRoot` and `ReadMapEnd`, which is also used to read Unreal's `TMap` properties. The difference is that UE's `TMap` is strictly typed but JSON object values can have arbitrary type. This means that if you use `FDcPipeVisitor` to pipe a `FDcJsonReader` into a `FDcPropertyWriter` it won't work.

The good news is that DataConfig data model is designed to support these use cases. As long as you can use `FDcReader/FDcWriter` API to describe the format you want to serialize you're good to go. Mapping and conversion between these different shapes of reader/writers are handled by [deserializers](./Deserializer.md).

Some additional caveats:

- Similar to Unreal's `TJsonReader`, we provide `TDcJsonReader` with 2 specializations:
    - `FDcJsonReader` that reads `TCHAR*, FString`
    - `FDcAnsiJsonReader` that reads `char*`.
- We're supporting a relaxed superset of JSON:
    - Allow C Style comments, i.e `/* block */` and `// line` .
    - Allow trailing comma, i.e `[1,2,3,],` .
    - Allow non object root. You can put a list as the root.
- String parsing and number parsing are delegated to Unreal's built-ins:
    - Parse string: `FParse::QuotedString()`
    - Parse numbers:  `TCString::Atof/Strtoi/Strtoi64`

## Utilities

Finally there're some utility reader/writers for various purposes.

- `FDcNoopWriter` - a writer that does literally nothing. Useful to benchmark reader performance.
- `FDcWeakCompositeWriter` - a writer that multiplex into a list of writers. Useful to trace writing calls.
- `FDcPutbackReader` - `FDcReader` doesn't support lookahead. It can only peek next item's type but not value. This class is used to support limited lookahead by *putting back* read value. It's used in implementing custom deserializer handlers.

## Conclusion

You should consider implement new `FDcReader/FDcWriter` when you want to support a new file format. You can also implement utility reader/writer that nest other reader/writers.