# Root Object

Many modern JSON like data markup languages allows root level object and arrays, i.e omitting top level braces. This can be done in DataConfig with [custom serialize handlers](../Programming/SerializerDeserializer.md#handlers).

* [DcSerDeRoot.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeRoot.h)
* [DcSerDeRoot.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeRoot.cpp)


```c++
// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeRoot.cpp
// root object
FString Str = TEXT(R"(

    "Name" : "Foo",
    "Id" : 253,
    "Type" : "Beta"

)");

//  equivalent fixture
FDcExtraSimpleStruct Expect;
Expect.Name = TEXT("Foo");
Expect.Id = 253;
Expect.Type = EDcExtraTestEnum1::Beta;
```


```c++
// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeRoot.cpp
// root list
FString Str = TEXT(R"(

    "Alpha",
    "Beta",
    "Gamma"

)");

//  equivalent fixture
TArray<EDcExtraTestEnum1> Expect = {
    EDcExtraTestEnum1::Alpha,
    EDcExtraTestEnum1::Beta,
    EDcExtraTestEnum1::Gamma};
```

This can be a tiny QOL improvement for manually authoring JSON data. 
