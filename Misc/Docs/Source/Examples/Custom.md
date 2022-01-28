# Custom Deserialization

DataConfig support custom serialization and deserialization logic by implementing `FDcSerializeDelegate/FDcDeserializeDelegate`.

In this example We'd like to deserialize `FColor` from `#RRGGBBAA`. First you'll need to implement a  `FDcDeserializePredicate` delegate to pick out `FColor`:

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
    return DcDeserializeUtils::PredicateIsUStruct<FColor>(Ctx);
}
```

Then we'll need to implement a `FDcDeserializeDelegate` to deserialize a `FColor`. Here we'll do it by writing through `R/G/B/A` fields by name with the `FDcWriter` API.

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
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

On deserialization we'll need to register these pair of delegates to the `FDcDeserializer`.

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
FDcDeserializer Deserializer;
DcSetupJsonDeserializeHandlers(Deserializer);
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
    FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);
```

And then it's done. It would work recursively on `FColor` everywhere, like in `UCLASS/USTRUCT` members, in `TArray/TSet` and in `TMap` as key or values.

Note that DataConfig completely seperate serialization and deserialization logic. To serialize `FColor` into `#RRGGBBAA` string one needs to implement similar set of methods.
