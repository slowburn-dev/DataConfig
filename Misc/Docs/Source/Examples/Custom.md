# Custom Serialization And Deserialization

DataConfig support custom serialization and deserialization logic by implementing `FDcSerializeDelegate/FDcDeserializeDelegate`.

In this example, we'd like to convert `FColor` into `#RRGGBBAA` and vice versa: 

* [DcSerdeColor.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeColor.h)
* [DcSerdeColor.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/Deserialize/DcSerDeColor.h
USTRUCT()
struct FDcExtraTestStructWithColor1
{
    GENERATED_BODY()

    UPROPERTY() FColor ColorField1;
    UPROPERTY() FColor ColorField2;
};

// DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeColor.cpp
FString Str = TEXT(R"(
    {
        "ColorField1" : "#0000FFFF",
        "ColorField2" : "#FF0000FF",
    }
)");
```

First you'll need to implement a `FDcDeserializePredicate` delegate to pick out `FColor` properties:

```c++
//  DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
    return DcDeserializeUtils::PredicateIsUStruct<FColor>(Ctx);
}
```

Then we'll need to implement a `FDcDeserializeDelegate` to deserialize a `FColor`. Here we'll do it by writing through `R/G/B/A` fields by name with the `FDcWriter` API.

```c++
// DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeColor.cpp
FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

    FString ColorStr;
    DC_TRY(Ctx.Reader->ReadString(&ColorStr));
    FColor Color = FColor::FromHex(ColorStr);

    FColor* ColorPtr = (FColor*)Datum.DataPtr;
    *ColorPtr = Color;

    return DcOk();
}
```

Note how we retrieve the hex string, then parse it with `FColor::FromHex`.

Upon deserializing we'll need to register these pair of delegates to the `FDcDeserializer`.

```c++
// DataConfigTests/Private/DcTestBlurb.cpp
FDcDeserializer Deserializer;
DcSetupJsonDeserializeHandlers(Deserializer);
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
    FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);
```

And then it's done! It would work recursively on `FColor` everywhere, like in `UCLASS/USTRUCT` members, in `TArray/TSet` and in `TMap` as key or values.

Note that DataConfig completely separate serialization and deserialization logic. To serialize `FColor` into `#RRGGBBAA` string one needs to implement a similar set of methods.
