# Nested Arrays

UE property system has a limitation that you can't have a property of nested array:

```c++
// UHT would error out: The type 'TArray<int32>' can not be used as a value in a TArray  
UPROPERTY() TArray<TArray<int>> Arr2D;
```

You can workaround this by wrap inner array in a struct. DataConfig however is flexible enough that you can serialize and deserialize nested JSON arrays into your data structures however you want:

* [DcSerDeNested.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeNested.h)
* [DcSerDeNested.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeNested.cpp)

```c++
// DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeNested.h
USTRUCT()
struct FDcExtraTestNested_Vec2
{
    GENERATED_BODY()

    UPROPERTY() TArray<FVector2D> Vec2ArrayField1;
    UPROPERTY() TArray<FVector2D> Vec2ArrayField2;
};

// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeNested.cpp
// equivalent fixture
FString Str = TEXT(R"(
    {
        "Vec2ArrayField1" :
        [
            {"X": 1.0, "Y": 2.0},
            {"X": 2.0, "Y": 3.0},
            {"X": 3.0, "Y": 4.0},
            {"X": 4.0, "Y": 5.0},
        ],
        "Vec2ArrayField2" :
        [
            [1.0, 2.0],
            [2.0, 3.0],
            [3.0, 4.0],
            [4.0, 5.0],
        ],
    }
)");
```

The next example we define a simple struct `FDcGrid2D` and stores the dimension using metadata.


```c++
// DataConfigExtra/Public/DataConfig/Extra/SerDe/DcSerDeNested.h
USTRUCT()
struct FDcGrid2D
{
    GENERATED_BODY()
    UPROPERTY() TArray<int> Data;
};

USTRUCT()
struct FDcExtraTestNested_Grid
{
    GENERATED_BODY()
    UPROPERTY(meta=(DcWidth = 2, DcHeight = 2)) FDcGrid2D GridField1;
    UPROPERTY(meta=(DcWidth = 3, DcHeight = 4)) FDcGrid2D GridField2;
};

// DataConfigExtra/Private/DataConfig/Extra/SerDe/DcSerDeNested.cpp
// equivalent fixture
FString Str = TEXT(R"(
    {
        "GridField1" :
        [
            [1,2],
            [3,4],
        ],
        "GridField2" :
        [
            [ 1, 2, 3],
            [ 4, 5, 6],
            [ 7, 8, 9],
            [10,11,12],
        ],
    }
)");
```

When data dimension doesn't match a diagnostic would be reported:

```
# DataConfig Error: Nested Grid2D height mismatch, Expect '2'
- [WideCharDcJsonReader] --> <in-memory>7:22
   5 |                    [1,2],
   6 |                    [1,2],
   7 |                    [1,2],
     |                    ^
   8 |                ],
   9 |            }
```
