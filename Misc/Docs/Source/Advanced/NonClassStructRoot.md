# Non Class/Struct Root

Sometimes you just want to deserialize something into an `TArray/TMap/TSet`. Then you'll realize that you don't have something corresponding to `StaticClass()/StaticStruct()` as root to pass to DataConfig.

```c++
// DataConfigTests/Private/DcTestBlurb2.cpp
FString Fixture = TEXT("[1,2,3,4,5]");
TArray<int> Arr;
```

Turns out you can create adhoc `FProperty` without `USTRUCT/UCLASS` parents and use them just fine. In DataConfig we've provided `DcPropertyUtils::FDcPropertyBuilder` to ease this use case.

* [DcPropertyUtils.h]({{SrcRoot}}DataConfigCore/Public/DataConfig/Property/DcPropertyUtils.h)

```c++
// DataConfigTests/Private/DcTestBlurb2.cpp
//  create int array property
using namespace DcPropertyUtils;
auto ArrProp = FDcPropertyBuilder::Array(
    FDcPropertyBuilder::Int()
    ).LinkOnScope();

FDcJsonReader Reader{Fixture};
DC_TRY(DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(ArrProp.Get(), &Arr)));

//  validate results
check(Arr.Num() == 5);
check(Arr[4] == 5);
```

Note that `FDcPropertyBuilder` would create a heap allocated `FProperty` and `LinkOnScope()` returns a `TUniquePtr`. You might want to cache the properties if used repeatly.





