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

## Coercion

Readers implements `FDcReader::Coercion()` which can be used to query if the next value can be coerced into other types.

Here's an example of reading a JSON number as string:

```c++
// DataConfigTests/Private/DcTestBlurb2.cpp
FDcJsonReader Reader(TEXT(R"(
    1.234567
)"));

//  check coercion
//  note this is a query and doesn't affect reading at all
bool bCanCoerceToStr;
DC_TRY(Reader.Coercion(EDcDataEntry::String, &bCanCoerceToStr));
check(bCanCoerceToStr);

//  read number token as stjring
//  note here we skipped parsing the number to float
FString NumStr;
DC_TRY(Reader.ReadString(&NumStr));

check(NumStr == TEXT("1.234567"));
return DcOk();
```

What's cool here is that `FDcJsonReader` does parsing at the `ReadFloat()` callsite. So when doing coercion above it actually skipped string to float parsing.

Here's a table of all existing coercion rules:

| Reader              | From Type  | To Type                                                      |
| ------------------- | ---------- | ------------------------------------------------------------ |
| `FDcPropertyReader` | Array      | Blob                                                         |
|                     | StructRoot | Blob                                                         |
| `FDcJsonReader`     | Double     | String                                                       |
|                     | Double     | Int8/Int16/Int32/Int64<br />UInt8/Uint16/UInt32/Uint64<br />Float |
|                     | String     | Name/Text                                                    |
| `FDcMsgPackReader`  | String     | Name/Text                                                    |

Some caveats regarding coercion:

* When reading from JSON/MsgPack string can be read as Name/Text for convenient.
* When reading from Property, Array/Struct can be read as a `FDcBlobViewData` which directly points to the memory span.

