# Writer API Alternatives

Previously we're deserializing `FColor` by writing into its member fields separately, which is a bit cumbersome. In this case DataConfig do support better alternatives.

Since we know that `FColor` is POD type we can construct one by filling in correct bit pattern. In this case `FDcPropertyWriter` allow struct property to be coerced from a blob:

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriteBlob>(const FColor& Color, FDcDeserializeContext& Ctx)
{
    return Ctx.Writer->WriteBlob({
        (uint8*)&Color, // treat `Color` as opaque blob data
        sizeof(FColor)
    });
}
```

Alternatively we can get `FProperty` and data pointer in place and setting the value through Unreal's `FProperty` API:

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriteDataEntry>(const FColor& Color, FDcDeserializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

    Datum.CastFieldChecked<FStructProperty>()->CopySingleValue(Datum.DataPtr, &Color);
    return DcOk();
}
```

Note that we already know that `Datum.DataPtr` points to a allocated `FColor` instance. Thus we can simply cast it into a `FColor*` and directly manipulate the pointer.

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WritePointer>(const FColor& Color, FDcDeserializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

    FColor* ColorPtr = (FColor*)Datum.DataPtr;
    *ColorPtr = Color;  // deserialize by assignment

    return DcOk();
}
```
Note that these techniques also applies on serialization:

```c++
//  DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeColor.cpp
FDcResult HandlerColorSerialize(FDcSerializeContext& Ctx)
{
    FDcPropertyDatum Datum;
    DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

    FColor* ColorPtr = (FColor*)Datum.DataPtr;
    DC_TRY(Ctx.Writer->WriteString(TEXT("#") + ColorPtr->ToHex()));

    return DcOk();
}
```
