# Reader/Writer

FDcReader/FDcWriter defines the set of API for accessing DataConfig data model. Here's a check list for implementing a reader/writer:

1. Implement `GetID()/ClassID()` for RTTI.
2. Implement `PeekRead()/PeekWriter()` and selected set of `ReadXXX()/WriteXXX()`.
3. Implement `FormatDiagnostic()` for error reporting.

You should look at builtin implementation for references. Here's some general rules and caveats:

- `PeekRead()/PeekWrite()` should act like it's side-effect free.

   This means that it's OK to call `PeekRead()/PeekWrite()` multiple times. In comparison access methods like `ReadBool()/WriteBool()` consume the data and alternate internal state. Note that under the hood it might do anything. Both returns `FDcResult` so the peek can fail. The reason behind this is that calling `PeekRead()/PeekWrite()` is totally optional. In `FDcJsonReader::PeekRead()` we do parsing and cache the parsed result to follow this convention.

- `CastByID()` does not respect inheritance hierarchy.

   We have this very minimal RTTI implemetantion that only allow casting to the exact type.

- Implement a subset of the data model.

  The API is designed in a way that it covers the whole Property System. It's also a super set that can express common formats like JSON/MsgPack. For example JSON don't have struct, class or set. It's actually the job of Serializer/Deserializer to convert between these subsets to the property system.

## Builtin Reader/Writer

We have 3 major pairs of reader/writers:

* `FDcPropertyReader/FDcPropertyWriter` - Accesing Unreal Engin property system.
* `FDcJsonReader/FDcJsonWriter` - JSON support.
* `FDcMsgPackReader/FDcMsgPackWriter` - MsgPack support.

These are all talked about in details in the [formats section][1]. We'll go through other builtin Reader/Writers below.

## `FDcPipeVisitor` and `FDcPrettyPrintWriter`

`FDcPipeVisitor` takes a `FDcReader` and a `FDcWriter` then start peek-read-write loop until it peeks `EDcDataEntry::Ended` from reader or an error happens 

Then there's `FDcPrettyPrintWriter` that dumps everything that got write to it as string.

Combining these two we get a way to dump arbitrary `FDcReader` into a string!. This is how built-in debug dump features are implemented:

```c++
// DataConfigCore/Private/DataConfig/Automation/DcAutomationUtils.cpp
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

`FDcPipeVisitor` is a handy utility that we use it extensively through the code base for various cases. Try `FDcPipeVisitor` when you got a reader/writer pair.

There's also `FNoopWriter` takes every write and do nothing with it.

## Composition

Reader/Writers can also be composited and nested:

* `FDcWeakCompositeWriter` is a writer that multiplex into a list of writers. You can combine an arbitrary writer with a `FPrettyPrintWriter` then get a tracing writer.
* `FDcPutbackReader/FPutbackWriter`: Reader/writers don't support lookahead. It can only peek next item's type but not value. This class is used to support limited lookahead by *putting back* read value. We'll see it being used in implementing custom deserializer handlers.

## Conclusion

Implement new `FDcReader/FDcWriter` when you want to support a new file format. You can also write utility reader/writer that composite existing ones.

[1]:../Formats/index.html "Formats"












