# Inline Struct

In [AnyStruct](AnyStruct.md) we implemented `FDcAnyStruct` that can store arbitrary heap allocated `USTRUCT`. A shortcoming of this approach is that it introduces overhead of heap allocated memory, which also have worse cache locality comparing to stack allocated structs.

In this example we implemented a series of structs `FDcInlineStruct64/FDcInlineStruct128/FDcInlineStruct256/FDcInlineStruct512` which stores a USTRUCT inline. Think it as a small buffer optimized version of `FDcAnyStruct`. These can also be used as a cheap alternative to UCLASS based polymorphism.

* [DcInlineStruct.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Types/DcInlineStruct.h)
* [DcInlineStruct.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Types/DcInlineStruct.cpp)

```c++
// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeInlineStruct.cpp
//  stack allocated usage
FDcInlineStruct64 Inline1;
Inline1.Emplace<FColor>(255, 0, 0, 255);
UTEST_TRUE("Inline Struct", *Inline1.GetChecked<FColor>() == FColor::Red);

//  support copy 
FDcInlineStruct64 Inline2 = Inline1;
UTEST_TRUE("Inline Struct", *Inline2.GetChecked<FColor>() == FColor::Red);
```

Serialization handlers for inline structs are similar to any struct ones:

* [DcSerDeInlineStruct.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeInlineStruct.h)
* [DcSerDeInlineStruct.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeInlineStruct.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeInlineStruct.h
USTRUCT()
struct FDcExtraTestWithInlineStruct1
{
    GENERATED_BODY()

    UPROPERTY() FDcInlineStruct64 InlineField1;
    UPROPERTY() FDcInlineStruct64 InlineField2;;
};

// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeInlineStruct.cpp
FString Str = TEXT(R"(
    {
        "InlineField1" : {
            "$type" : "DcExtraTestSimpleStruct1",
            "NameField" : "Foo"
        },
        "InlineField2" : {
            "$type" : "DcExtraTestStructWithColor1",
            "ColorField1" : "#0000FFFF",
            "ColorField2" : "#FF0000FF"
        }
    }
)");
```

Note that one limitation with inline structs the USTRUCT get put in must have smaller size than the inline struct storage. Deserialize handlers would check for these cases:

```
LogDataConfigCore: Display: # DataConfig Error: Inline struct too big: BufSize '56', Struct 'DcExtraTestStruct128' Size '128'
LogDataConfigCore: Display: - [WideCharDcJsonReader] --> <in-memory>5:38
   3 |        {
   4 |            "InlineField1" : {
   5 |                "$type" : "DcExtraTestStruct128",
     |                                                ^
   6 |                "NameField" : "Foo"
   7 |            },
LogDataConfigCore: Display: - [DcPropertyWriter] Writing property: (FDcExtraTestWithInlineStruct1)$root.(FDcInlineStruct64)InlineField1
LogDataConfigCore: Display: - [DcSerializer]
```




