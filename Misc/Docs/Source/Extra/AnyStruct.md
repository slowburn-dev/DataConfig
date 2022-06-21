# AnyStruct

This is an intermediate example that takes advantage of the flexibility provided by the property system. `FDcAnyStruct` is a struct that stores a heap allocated `USTRUCT` of any type while maintaining value semantic on itself. If you're familiar with the concept of variant type, just think of it as a variant type that supports all `USTRUCT`:

* [DcAnyStruct.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Types/DcAnyStruct.h)
* [DcAnyStruct.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Types/DcAnyStruct.cpp)


```c++
// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeAnyStruct.cpp
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

We then implemented conversion logic between `FDcAnyStruct` and JSON:

* [DcSerDeAnyStruct.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeAnyStruct.h)
* [DcSerDeAnyStruct.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeAnyStruct.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeAnyStruct.h
USTRUCT()
struct FDcExtraTestWithAnyStruct1
{
    GENERATED_BODY()

    UPROPERTY() FDcAnyStruct AnyStructField1;
    UPROPERTY() FDcAnyStruct AnyStructField2;
    UPROPERTY() FDcAnyStruct AnyStructField3;
};

// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeAnyStruct.cpp
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
```

Note how the custom `FColor <-> "#RRGGBBAA"` conversion recursively works within `FDcAnyStruct`. This should be a good starting point for you to implement your own nested variant types and containers. For more details refer to the implementation of `HandlerDcAnyStruct[Serialize/Deserialize]`. 