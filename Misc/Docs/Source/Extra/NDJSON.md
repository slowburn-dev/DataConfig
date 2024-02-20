# NDJSON

[NDJSON][1] is a popular extension to JSON that stores 1 JSON object per line. DataConfig's JSON parser and writer is flexible enough to easily support this use case.

* [DcNDJSON.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Misc/DcNDJSON.h)
* [DcNDJSON.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Misc/DcNDJSON.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/Misc/DcNDJSON.h
template<typename TStruct>
DATACONFIGEXTRA_API FDcResult LoadNDJSON(const TCHAR* Str, TArray<TStruct>& Arr)
// ...
template<typename TStruct>
DATACONFIGEXTRA_API FDcResult SaveNDJSON(const TArray<TStruct>& Arr, FString& OutStr)
// ...
```

With this method we can load a NDJSON string into a struct array and later serialize it back to NDJSON.

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Misc/DcNDJSON.cpp
FString Str = TEXT(R"(

    { "Name" : "Foo", "Id" : 1, "Type" : "Alpha" }
    { "Name" : "Bar", "Id" : 2, "Type" : "Beta" }
    { "Name" : "Baz", "Id" : 3, "Type" : "Gamma" }

)");

UTEST_OK("Extra NDJSON", LoadNDJSON(*Str, Dest));

FString SavedStr;
UTEST_OK("Extra NDJSON", SaveNDJSON(Dest, SavedStr));
```

Note that our parser [supports common extension to JSON](../Formats/JSON.md#json-reader):

- Allow C Style comments, i.e `/* block */` and `// line` .
- Allow trailing comma, i.e `[1,2,3,],` .
- Allow non object root. You can put a list as the root, or even string, numbers.

[1]: http://ndjson.org/
