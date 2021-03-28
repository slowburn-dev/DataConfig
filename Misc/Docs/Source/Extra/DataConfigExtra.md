# DataConfigExtra

Module `DataConfigExtra` contains examples that doesn't dependent on `Engine/UnrealEd`.

## Deserialize JSON into Struct

There's a built-in method in `JsonUtilities` module that simply deserialize a JSON string into a struct. In this example we implemented a similar method with almost identical API:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcJsonConverter.cpp
FString Str = TEXT(R"(
    {
        "StrField" : "Foo",
        "IntField" : 253,
        "BoolField" : true
    }
)");

FDcTestJsonConverter1 Lhs;
bool LhsOk = DcExtra::JsonObjectStringToUStruct(Str, &Lhs);

FDcTestJsonConverter1 Rhs;
bool RhsOk = FJsonObjectConverter::JsonObjectStringToUStruct(Str, &Rhs, 0, 0);
```

Comparing to the stock method `DcExtra::JsonObjectStringToUStruct` allows relaxed JSON with comments and trailing comma. It would also provide better diagnostics on parse error.

## Deserialize `FColor`

This has been shown multiple times in previous chapters. It's also a benchmark use case for our custom deserialization logic:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeColor.cpp
FString Str = TEXT(R"(
    {
        "ColorField1" : "#0000FFFF",
        "ColorField2" : "#FF0000FF",
    }
)");

// deserialized equivelent
FDcExtraTestStructWithColor1 Expect;

Expect.ColorField1 = FColor::Blue;
Expect.ColorField2 = FColor::Red;
```

## Deserialize Base64 string as Blob 

In this example we deserialize `TArray<uint8>` from Base64 strings in JSON:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeBase64.cpp
FString Str = TEXT(R"(
    {
        "BlobField1" : "dGhlc2UgYXJlIG15IHR3aXN0ZWQgd29yZHM=",
        "BlobField2" : "",
    }
)");

// deserialized equivelent
FDcExtraTestStructWithBase64 Expect;

const char* Literal = "these are my twisted words";
Expect.BlobField1 = TArray<uint8>((uint8*)Literal, FCStringAnsi::Strlen(Literal));
Expect.BlobField2 = TArray<uint8>();
```

In the predicate we are checking for `Array`s with custom meta data `DcExtraBlob`:

```c++
// DataConfig\Source\DataConfigExtra\Public\DataConfig\Extra\Deserialize\DcDeserializeBase64.h
USTRUCT()
struct FDcExtraTestStructWithBase64
{
    GENERATED_BODY()

    UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField1;
    UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField2;
};
```

UE support arbitrary meta data in the `meta = ()` segment. But beware that the meta data is only available when `WITH_EDITORDATA` flag is defined.

## Deserialize `FDcAnyStruct`

we've implemented `FDcAnyStruct` that can be used to store a heap allocated `USTRUCT` of any type while keep proper value sematic on itself:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.cpp
//  instantiate from heap allocated structs
FDcAnyStruct Any1 = new FDcExtraTestSimpleStruct1();
Any1.GetChecked<FDcExtraTestSimpleStruct1>()->NameField = TEXT("Foo");

//  supports moving
FDcAnyStruct Any2 = MoveTemp(Any1);
check(!Any1.IsValid());
check(Any2.GetChecked<FDcExtraTestSimpleStruct1>()->NameField == TEXT("Foo"));
Any2.Reset();

//  supports shared referencing
Any2 = new FDcExtraTestSimpleStruct2();
Any2.GetChecked<FDcExtraTestSimpleStruct2>()->StrField = TEXT("Bar");

Any1 = Any2;

check(Any1.DataPtr == Any2.DataPtr);
check(Any1.StructClass == Any2.StructClass);
```

In this example we implemented predicate and handler to support deserializing `FDcAnyStruct` from a JSON object with a `$type` field, or a `null`.

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeAnyStruct.cpp
FString Str = TEXT(R"(
    {
        "AnyStructField1" : {
            "$type" : "DcExtraTestSimpleStruct1",
            "NameField" : "Foo"
        },
        "AnyStructField2" : {
            "$type" : "DcExtraTestStructWithColor1",
            "ColorField1" : "#0000FFFF",
            "ColorField2" : "#FF0000FF"
        },
        "AnyStructField3" : null
    }
)");

//...

UTEST_TRUE("...", Dest.AnyStructField1.GetChecked<FDcExtraTestSimpleStruct1>()->NameField == TEXT("Foo"));
UTEST_TRUE("...", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField1 == FColor::Blue);
UTEST_TRUE("...", Dest.AnyStructField2.GetChecked<FDcExtraTestStructWithColor1>()->ColorField2 == FColor::Red);
UTEST_TRUE("...", !Dest.AnyStructField3.IsValid());
```

Note how custom `FColor` deserializing works inside a `FDcAnyStruct`.

## Copying struct while renaming field names

This is an example of using `FDcDeserializer` with non `FDcJsonReader`. It uses `FDcPropertyReader` with the `DcPropertyPipeHandlers` to do renaming:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeRenameStructFieldNames.cpp
//  struct equivelent to this:
FString Str = TEXT(R"(
    {
        "FromName1" : "Foo",
        "FromStructSet1" : 
        [
            {
                "FromStr1" : "One",
                "FromInt1" : 1,
            },
            {
                "FromStr1" : "Two",
                "FromInt1" : 2,
            }
        ]
    }
)");

// ... deserialize with a functor renaming `FromXXX` to `ToXXX`:
UTEST_OK("...", DcExtra::DeserializeStructRenaming(FromDatum, ToDatum, FDcExtraRenamer::CreateLambda([](const FName& FromName){
    FString FromStr = FromName.ToString();
    if (FromStr.StartsWith(TEXT("From")))
        return FName(TEXT("To") + FromStr.Mid(4));
    else
        return FromName;
})));

// ... results into a struct equivelent to this: 
FString Str = TEXT(R"(
    {
        "ToName1" : "Foo",
        "ToStructSet1" : 
        [
            {
                "ToStr1" : "One",
                "ToInt1" : 1,
            },
            {
                "ToStr1" : "Two",
                "ToInt1" : 2,
            }
        ]
    }
)");
```

## Access property by path

UE built-in module `PropertyPath` allow accessing nested object properties by a path like `Foo.Bar.Baz`:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcPropertyPathAccess.cpp
FString Str;
UTEST_TRUE("...", PropertyPathHelpers::GetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), Str));
UTEST_TRUE("...", Str == TEXT("Foo"));

UTEST_TRUE("...", PropertyPathHelpers::SetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), FString(TEXT("Bar"))));
UTEST_TRUE("...", Outer->StructRoot.Middle.InnerMost.StrField == TEXT("Bar"));
```

We implemented a pair of methods `GetDatumPropertyByPath/SetDatumPropertyByPath` with `FDcPropertyReader`:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Types/DcPropertyPathAccess.cpp
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField"), TEXT("Foo")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField"), TEXT("Bar0")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField"), TEXT("Bar1")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField"), TEXT("FooValue")));

UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField", TEXT("AltFoo")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField", TEXT("AltBar0")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField", TEXT("AltBar1")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField", TEXT("AltFooValue")));
```

Comparing to `PropertyPathHelpers` these new ones support `Array` and `Map`, and support `USTRUCT` roots. We're also missing some features like expanding weak/lazy object references but it should be easy to implement.

