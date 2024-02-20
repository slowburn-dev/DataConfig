# Optional

**This example only works with UE5.4+**

UE 5.4 introduced optional property which now allows you to mark `TOptional` fields as `UPROPERTY`:

```c++
// DataConfigTests54/Private/DcTestUE54.h
USTRUCT()
struct FDcTestOptional
{
    GENERATED_BODY()

    UPROPERTY() TOptional<float> OptionalFloatField1;
    UPROPERTY() TOptional<float> OptionalFloatField2;

    UPROPERTY() TOptional<FString> OptionalStrField1;
    UPROPERTY() TOptional<FString> OptionalStrField2;

    UPROPERTY() TOptional<FDcInnerStruct54> OptionalStructField1;
    UPROPERTY() TOptional<FDcInnerStruct54> OptionalStructField2;
};
```

It's a useful utility to correctly model your data structure. For example you can use `TOptional<uint32>` to correctly represent an optional unsigned int, rather than using a special value like `UINT32_MAX`. DataConfig fully supports optional in the reader writer API:

```c++
// DataConfigTests54/Private/DcTestUE54.cpp
TOptional<FString> Source;

// ...
DC_TRY(Writer.WriteOptionalRoot());                 // Optional Root
DC_TRY(Writer.WriteString(TEXT("Some String")));    // Str
DC_TRY(Writer.WriteOptionalEnd());                  // Optional End
// ...
FString ReadStr;
DC_TRY(Reader.ReadOptionalRoot());      // Optional Root
DC_TRY(Reader.ReadString(&ReadStr));    // Str
check(ReadStr == TEXT("Some String"));  
DC_TRY(Reader.ReadOptionalEnd());       // Optional End
// ...
DC_TRY(Writer.WriteOptionalRoot()); //  Optional Root
DC_TRY(Writer.WriteNone());         //  None
DC_TRY(Writer.WriteOptionalEnd());  //  Optional End
// ...
DC_TRY(Reader.ReadOptionalRoot());  // Optional Root
DC_TRY(Reader.ReadNone());          // None
DC_TRY(Reader.ReadOptionalEnd());   // Optional End
```

Optional also maps perfectly with JSON since any JSON value can be `null`:

```c++
// DataConfigTests54/Private/DcTestUE54.cpp
FString Str = TEXT(R"(

    {
        "OptionalFloatField1" : 17.5,
        "OptionalFloatField2" : null,

        "OptionalStrField1" : "Alpha",
        "OptionalStrField2" : null,

        "OptionalStructField1" : {
            "StrField" : "Beta",
            "IntField" : 42
        },
        "OptionalStructField2" : null
    }

)");

//  equivalent fixture
FDcTestOptional Expect;
Expect.OptionalFloatField1 = 17.5f;
Expect.OptionalStrField1 = TEXT("Alpha");
Expect.OptionalStructField1.Emplace();
Expect.OptionalStructField1->StrField = TEXT("Beta");
Expect.OptionalStructField1->IntField = 42;
```

Note how it works with nested optional struct.

