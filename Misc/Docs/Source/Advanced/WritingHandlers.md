# Tips for writing handlers

There're some recurring patterns when writing handlers in DataConfig.

## Recursive Deserialize

When deserializing a container like `USTRUCT` root or `TArray` you'll need to recursively deserialize children properties. This is wrapped in a single function call:

```c++
// DataConfigCore/Private/DataConfig/Deserialize/Handlers/Json/DcJsonStructDeserializers.cpp
DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
```
Internally it would push writer's next property into `FDcDeserializeContext::Properties` to satisfiy the invariant that `FDcDeserializeContext::TopProperty()` always points to the current writing property. It would also clear up the top property on return.

Another example is how we pipe deserialize a `TMap<>`. When at key and value position we simply call this method 2 times:

```C++
// DataConfigCore/Private/DataConfig/Deserialize/Handlers/Property/DcPropertyPipeDeserializers.cpp
DC_TRY(Ctx.Reader->ReadMapRoot());
DC_TRY(Ctx.Writer->WriteMapRoot());

EDcDataEntry CurPeek;
while (true)
{
    DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
    if (CurPeek == EDcDataEntry::MapEnd)
        break;

    DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
    DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
}

DC_TRY(Ctx.Reader->ReadMapEnd());
DC_TRY(Ctx.Writer->WriteMapEnd());
```

## Provide `TopObject()`

Sometimes deserialization will create new `UObject` along the way. In this case you'll need to fill in `FDcDeserializeContext::Objects` so the top one is used for `NewObject()` calls. For transient objecst you can use `GetTransientPackage()`:

```c++
// DataConfigTests/Private/DcTestDeserialize.cpp
Ctx.Objects.Push(GetTransientPackage());
```

## Peek By Value

Sometimes you want to peek the content of the next entry. For example in `DcExtra::HandlerBPDcAnyStructDeserialize()` we're dealing with a JSON like this:

```json
// DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
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
// DataConfigEditorExtra/Private/DataConfig/EditorExtra/Deserialize/DcDeserializeBPClass.cpp
FDcPutbackReader PutbackReader(Ctx.Reader);
PutbackReader.Putback(EDcDataEntry::MapRoot);
TDcStoreThenReset<FDcReader*> RestoreReader(Ctx.Reader, &PutbackReader);

FDcScopedProperty ScopedValueProperty(Ctx);
DC_TRY(ScopedValueProperty.PushProperty());
DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
```

Beware that `Putback` only support a limited subset of data types.
