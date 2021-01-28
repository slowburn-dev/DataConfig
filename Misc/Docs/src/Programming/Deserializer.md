# Deserializer

In the [previous chapter](./BuiltinReaderWriters.md) we mentioned that DataConfig data model is a super set of the property system. `FDcDeserializer` is used to convert different subsets of it into the property system.

## Context

A company class to the deserializer is `FDcDeserializeContext`:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Deserialize/DcDeserializeTypes.h
struct DATACONFIGCORE_API FDcDeserializeContext
{
    //...
    FDcDeserializer* Deserializer;
    FDcReader* Reader;
    FDcPropertyWriter* Writer;
    //...
};
```
Comparing to `FDcPipeVisitor` which takes a `FDcReader` and a `FDcWriter`, `FDcDeserializeContext` takes explicitly a `FDcPropertyWriter` to construct. The deserializer reads from arbitrary reader but writes only into the property system objects.

## Handlers 

Custom deserialize logic is provided through `FDcDeserializeDelegate`:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Deserialize/DcDeserializeTypes.h
using FDcDeserializeDelegateSignature = FDcResult(*)(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet);
DECLARE_DELEGATE_RetVal_TwoParams(FDcResult, FDcDeserializeDelegate, FDcDeserializeContext&, EDcDeserializeResult&);

```

We call these functions **Handlers**. Here's a simple one that deserialize booleans:

```c++
// DataConfig/Source/DataConfigCore/Private/DataConfig/Deserialize/Handlers/Json/DcJsonPrimitiveDeserializers.cpp
FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
    if (!Ctx.TopProperty().IsA<FBoolProperty>())
    {
        return DcOkWithFallThrough(OutRet);
    }

    EDcDataEntry Next;
    DC_TRY(Ctx.Reader->PeekRead(&Next));

    if (Next != EDcDataEntry::Bool)
    {
        return DC_FAIL(DcDDeserialize, DataEntryMismatch)
            << EDcDataEntry::Bool << Next;
    }

    bool Value;
    DC_TRY(Ctx.Reader->ReadBool(&Value));
    DC_TRY(Ctx.Writer->WriteBool(Value));

    OutRet = EDcDeserializeResult::Success;
    return DcOkWithProcessed(OutRet);
}
```

The body of handlers basically follows the same pattern:

- Start by checking context for precondition:
    - `Ctx.TopProperty()` which points to the current writing property.
    - Peek `Ctx.Reader` state to see if it's reading the correct data type.
- If any precondition failed return `DcOkWithFallThrough(OutRet)`. Deserializer will try others if possible.
- If all precondition passed then do the read and writing calls and finally return `DcOkWithProcessed(OutRet)`.

## Predicates

In many occasions we want to provide custom deserialization logic for a very specific class. The selection process is done through `FDcDeserializePredicate`:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Deserialize/DcDeserializeTypes.h
using FDcDeserializePredicateSignature = EDcDeserializePredicateResult(*)(FDcDeserializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(EDcDeserializePredicateResult, FDcDeserializePredicate, FDcDeserializeContext&);
```

We call these `Predicates`. Here's an example of selecting the bulit-in `FColor`:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeColor.cpp
EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
    UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
    return Struct && Struct == TBaseStructure<FColor>::Get()
        ? EDcDeserializePredicateResult::Process
        : EDcDeserializePredicateResult::Pass;
}
```

Similar to handlers it's checking `Ctx.TopProperty()` and return a `EDcDeserializePredicateResult` to decide to process or pass.

## Deserializer

Finally there's the `FDcDeserializer` which is just a collection of predicates and handlers. It contains no mutable state as those are put in `FDcDeserializeContext`:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Deserialize/DcDeserializer.h
struct DATACONFIGCORE_API FDcDeserializer : public FNoncopyable
{
    //...
    FDcResult Deserialize(FDcDeserializeContext& Ctx);

    void AddDirectHandler(FFieldClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
    void AddDirectHandler(UClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
    void AddPredicatedHandler(FDcDeserializePredicate&& Predicate, FDcDeserializeDelegate&& Delegate);
    //...
};
```

`AddDirectHandler()` registers handlers for a specific property type. Here's an example of registering the `HandlerBoolDeserialize` above:

```c++
// DataConfig/Source/DataConfigCore/Private/DataConfig/Deserialize/DcDeserializerSetup.cpp
Deserializer.AddDirectHandler(FBoolProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerBoolDeserialize));
```

`AddPredicatedHandler()` registers a predicate and handler pair. Here's an example of registering the `PredicateIsColorStruct` predicate above:

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
    FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
);
```

To start deserialization you need to prepare a `FDcDeserializeContext` and call `FDcDeserializer::Deserialize(Ctx)`:


```c++
// DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
//  prepare context for this run
FDcPropertyDatum Datum(FDcTestExampleStruct::StaticStruct(), &Dest);
FDcJsonReader Reader(Str);
FDcPropertyWriter Writer(Datum);

FDcDeserializeContext Ctx;
Ctx.Reader = &Reader;
Ctx.Writer = &Writer;
Ctx.Deserializer = &Deserializer;
Ctx.Properties.Push(Datum.Property);
DC_TRY(Ctx.Prepare());

//  kick off deserialization
DC_TRY(Deserializer.Deserialize(Ctx));
```

TODO: `TopObject/TopProperty`
TODO: actually we're missing a `TopObject` example.



