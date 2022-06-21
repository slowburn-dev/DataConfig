# MsgPack

[MsgPack][1] is an popular binary serialization format. It can be considered as a binary superset of JSON. Unreal Engine already supports [`Cbor`][2] module which is a format which is very similar to MsgPack.

We choose to implement MsgPack as we're more familiar with it and also providing an alternative.

## MsgPack Reader/Writer

For the most part MsgPack reader/writer works just like their [JSON counterpart](./Json.md). There're just a few additional data types that belongs to this:

### Binary

MsgPack directly supports `bin format family` which directly maps to `EDcDataEntry::Blob`:

```c++
// DataConfigTests/Private/DcTestBlurb.cpp
DC_TRY(Writer.WriteBlob({Bytes, 0}));
TArray<uint8> Arr = {1,2,3,4,5};

FDcMsgPackWriter Writer;
DC_TRY(Writer.WriteBlob(FDcBlobViewData::From(Arr)));
auto& Buf = Writer.GetMainBuffer();

FDcMsgPackReader Reader(FDcBlobViewData::From(Buf));
FDcBlobViewData Blob;
DC_TRY(Reader.ReadBlob(&Blob));

check(Blob.Num == 5);
check(FPlatformMemory::Memcmp(Arr.GetData(), Blob.DataPtr, Blob.Num) == 0);
```

### Extension

MsgPack also supports `ext format family` which is basically fixed size binary data with a header:

```c++
// DataConfigTests/Private/DcTestBlurb.cpp
FDcMsgPackWriter Writer;
DC_TRY(Writer.WriteFixExt2(1, {2, 3}));
auto& Buf = Writer.GetMainBuffer();

FDcMsgPackReader Reader(FDcBlobViewData::From(Buf));
uint8 Type;
FDcBytes2 Bytes;
DC_TRY(Reader.ReadFixExt2(&Type, &Bytes));

check(Type == 1);
check(Bytes.Data[0] == 2);
check(Bytes.Data[1] == 3);
```

## MsgPack Serialize/Deserialize

MsgPack handlers also support multiple setup types:

```c++
// DataConfigCore/Public/DataConfig/Serialize/DcSerializerSetup.h
enum class EDcMsgPackSerializeType
{
    Default,
    StringSoftLazy, // Serialize Soft/Lazy references as string
    InMemory,       // Serialize pointer/enum/FName etc as underlying integer values
};
```

### Persistent handlers 

The `Default` and `StringSoftLazy` options would setup a set of handlers that behaves
like their JSON counterparts. 

We have a "Property -> Json -> MsgPack -> Json -> Property" roundtrip test setup in `DataConfig.Core.RoundTrip.Property_Json_MsgPack_Json_Property` test.

### In Memory handlers

This is a special set of handlers that only makes sense for binary formats. For example pointers are serialized as memory addresses.

| EDcDataEntry        | Serialized                |
| ------------------- | ------------------------- |
| Name                | `[uint32, uint32, int32]` |
| Text                | `[void*, void*, uint32]`  |
| ObjectReference, ClassReference    | `void*`                   |
| SoftObjectReference, SoftClassReference | `FString or void*` |
| WeakObjectReference | `[int32, int32]` |
| LazyObjectReference | `<uuid as FIXEXT16>` |
| InterfaceReference | `[void*, void*]` |
| Delegate | `[int32, int32, (FName)[uint32, uint32, int32]]` |
| MulticastInlineDelegate, MulticastSparseDelegate | `[(list of <Delegate>)]` |
| FieldPath | `void*` |
| Enum | `uint64` |

With these handlers all data types can be serialized. Note that serializing stuff as memory address isn't always what you want. These are provided as soft of a reference on how to access various data.

[1]:https://msgpack.org/index.html "MsgPack"
[2]:https://docs.unrealengine.com/4.27/en-US/API/Runtime/Cbor "Cbor"
