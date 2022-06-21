# Access property by path

This example demonstrates that `FReader/FWriter` can be used standalone, without `FDcSerializer/FDcDeserializer`.

UE built-in module [`PropertyPath`][1] allow accessing nested object properties by a path like `Foo.Bar.Baz`:

* [DcPropertyPathAccess.h]({{SrcRoot}}DataConfigExtra/Public/DataConfig/Extra/Types/DcPropertyPathAccess.h)
* [DcPropertyPathAccess.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Types/DcPropertyPathAccess.cpp)

```c++
// DataConfigExtra/Private/DataConfig/Extra/Types/DcPropertyPathAccess.cpp
FString Str;
UTEST_TRUE("...", PropertyPathHelpers::GetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), Str));
UTEST_TRUE("...", Str == TEXT("Foo"));

UTEST_TRUE("...", PropertyPathHelpers::SetPropertyValue(Outer, TEXT("StructRoot.Middle.InnerMost.StrField"), FString(TEXT("Bar"))));
UTEST_TRUE("...", Outer->StructRoot.Middle.InnerMost.StrField == TEXT("Bar"));
```

We implemented a pair of methods `DcExtra::GetDatumPropertyByPath/SetDatumPropertyByPath` with `FDcPropertyReader`:

```c++
// DataConfigExtra/Private/DataConfig/Extra/Types/DcPropertyPathAccess.cpp
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField"), TEXT("Foo")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField"), TEXT("Bar0")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField"), TEXT("Bar1")));
UTEST_TRUE("...", CheckStrPtr(GetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField"), TEXT("FooValue")));

UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Middle.InnerMost.StrField", TEXT("AltFoo")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.0.StrField", TEXT("AltBar0")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.Arr.1.StrField", TEXT("AltBar1")));
UTEST_TRUE("...", SetDatumPropertyByPath<FString>(FDcPropertyDatum(Outer), "StructRoot.NameMap.FooKey.StrField", TEXT("AltFooValue")));
```

Comparing to `PropertyPathHelpers` these new ones support `Array` and `Map`, and support `USTRUCT` roots. We're missing some features like expanding weak/lazy object references but it should be easy to implement.

Remember that we have bundled JSON/MsgPack reader/writers that can also be used standalone.

[1]: https://docs.unrealengine.com/4.27/en-US/API/Runtime/PropertyPath "PropertyPath"