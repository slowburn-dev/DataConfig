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
using FDcDeserializeDelegateSignature = FDcResult(*)(FDcDeserializeContext& Ctx);
DECLARE_DELEGATE_RetVal_OneParam(FDcResult, FDcDeserializeDelegate, FDcDeserializeContext&);
```

We call these functions **handlers**. Here's a simple one that deserialize booleans:

```c++
// DataConfig/Source/DataConfigCore/Private/DataConfig/Deserialize/Handlers/Json/DcJsonPrimitiveDeserializers.cpp
FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx)
{
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

    return DcOk();
}
```

Note how we propagate errors to the caller by using `DC_TRY` or fail explicitly by returning `DC_FAIL`with diagnostic.

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
Deserializer.AddDirectHandler(
    FBoolProperty::StaticClass(), 
    FDcDeserializeDelegate::CreateStatic(HandlerBoolDeserialize)
);
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

## Tips for writing handlers

There're some recurring patterns when writing deserialization handlers in DataConfig.

### Recursive Deserialize

When deserializing a container like `USTRUCT` root or `TArray` you'll need to recursively deserialize children properties. Here's how it's done:

```c++
// DataConfig/Source/DataConfigCore/Private/DataConfig/Deserialize/Handlers/Json/DcJsonStructDeserializers.cpp
FDcScopedProperty ScopedValueProperty(Ctx);
DC_TRY(ScopedValueProperty.PushProperty());
DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
```
`FDcScopedProperty` is used to push writer's next property into `FDcDeserializeContext::Properties` to satisfiy the invariant that `FDcDeserializeContext::TopProperty()` always points to the current writing property.

### Provide `TopObject()`

Sometimes deserialization will create new `UObject` along the way. In this case you'll need to fill in `FDcDeserializeContext::Objects` so the top one is used for `NewObject()` calls. For transient objecst you can use `GetTransientPackage()`:

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestDeserialize.cpp
Ctx.Objects.Push(GetTransientPackage());
```

### Peek By Value

Sometimes you want to peek the content of the next entry. For example in `DcExtra::HandlerBPDcAnyStructDeserialize()` we're dealing with a JSON like this:

```json
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
{
    "AnyStructField1" : {
        "$type" : "/DataConfig/DcFixture/DcTestBlueprintStructWithColor",
        "NameField" : "Foo",
        //...
    }
}
```

We want to consume the `$type` key and its value, and then delegate the logic back to the deserializer. The solution here is first to consume the pair. Then we put back a `{`  then replace the reader:

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
FDcPutbackReader PutbackReader(Ctx.Reader);
PutbackReader.Putback(EDcDataEntry::MapRoot);
TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

FDcScopedProperty ScopedValueProperty(Ctx);
DC_TRY(ScopedValueProperty.PushProperty());
DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
```

Beware that `Putback` only support a limited subset of data types.
