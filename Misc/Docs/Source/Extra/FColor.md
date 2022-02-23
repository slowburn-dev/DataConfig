# FColor Serialization/Deserialization

This example has been shown in [previous chapter][1]. It's also a benchmark use case for our custom deserialization logic:

```c++
// DataConfig/Source/DataConfigExtra/Public/DataConfig/Extra/Deserialize/DcSerDeColor.h
USTRUCT()
struct FDcExtraTestStructWithColor1
{
    GENERATED_BODY()

    UPROPERTY() FColor ColorField1;
    UPROPERTY() FColor ColorField2;
};

// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeColor.cpp
FString Str = TEXT(R"(
    {
        "ColorField1" : "#0000FFFF",
        "ColorField2" : "#FF0000FF",
    }
)");
```

`FColor` is converted into a `#RRGGBBAA` hex string. The corresponding handlers looks pretty mirrored.

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeColor.cpp
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

FDcResult HandlerColorSerialize(FDcSerializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	FColor* ColorPtr = (FColor*)Datum.DataPtr;
	DC_TRY(Ctx.Writer->WriteString(TEXT("#") + ColorPtr->ToHex()));

	return DcOk();
}
```

Note how `FDcPropertyReader::ReadDataEntry` and `FDcPropertyWriter::WriteDataEntry` retrieves the next property as a `FDcPropertyDatum`, which allows us to directly manipulate a `FColor` pointer.

[1]: ../Examples/Custom.md "Custom"