# Pipe Deserialize

There's a set of deserialize handlers in `DcPropertyPipeHandlers` namespace. It's used to roundtripping property system objects.

Simply speaking it's equivalent to doing a `FDcPipeVisitor`  pipe visit.

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
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
    Ctx.Properties.Push(FromDatum.Property);
    DC_TRY(Ctx.Prepare());

    DC_TRY(Deserializer.Deserialize(Ctx));
}
```

These are provided as a set of basis to for building custom property wrangling utils. See `DataConfigExtra - DcDeserializeRenameStructFieldNames.h/cpp` for example.