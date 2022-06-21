# Field Renamer

DataConfig can also be used to author one-off utility. In this example we implemented `DcExtra::DeserializeStructRenaming()` that copies data between structural identical data structures, while renaming field names by a user function.

* [DcDeserializeRenameStructFieldNames.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Deserialize/DcDeserializeRenameStructFieldNames.h)
* [DcDeserializeRenameStructFieldNames.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeRenameStructFieldNames.cpp)

```c++
// DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeRenameStructFieldNames.cpp
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
UTEST_OK("...", DcExtra::DeserializeStructRenaming(
    FromDatum, ToDatum, FDcExtraRenamer::CreateLambda([](const FName& FromName){
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

This takes advantage of the `DcPropertyPipeHandlers` that simply do verbatim data piping. 

The gist is that you should consider DataConfig an option when working with batch data processing within Unreal Engine. We are trying to provide tools to support these use cases.