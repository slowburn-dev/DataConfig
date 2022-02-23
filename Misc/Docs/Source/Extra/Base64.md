# Base64 Blob Serialization/Deserialization

This demonstrates conversion between`TArray<uint8>` and Base64 encoded strings in JSON:

```c++
// DataConfig/Source/DataConfigExtra/Public/DataConfig/Extra/Deserialize/DcSerDeBase64.h
USTRUCT()
struct FDcExtraTestStructWithBase64
{
    GENERATED_BODY()

    UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField1;
    UPROPERTY(meta = (DcExtraBase64)) TArray<uint8> BlobField2;
};

// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeBase64.cpp
FString Str = TEXT(R"(
    {
        "BlobField1" : "dGhlc2UgYXJlIG15IHR3aXN0ZWQgd29yZHM=",
        "BlobField2" : "",
    }
)");
```

Note that we're tagging the `BlobField` with `(meta = (DcExtraBase64))` to explicitly show that we' want this member to be converted into Base64.

UE support [arbitrary meta data in the `meta = ()` segment][1]. But beware that the meta data is only available when `WITH_EDITORDATA` flag is defined. In predicate we check for this `DcExtraBase64` like this:

```c++
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcSerDeBase64.cpp
EDcDeserializePredicateResult PredicateIsBase64Blob(FDcDeserializeContext& Ctx)
{
	FArrayProperty* ArrayProperty = DcPropertyUtils::CastFieldVariant<FArrayProperty>(Ctx.TopProperty());

	//	check for only TArray<uint8>
	if (ArrayProperty == nullptr)
		return EDcDeserializePredicateResult::Pass;
	if (!ArrayProperty->Inner->IsA<FByteProperty>())
		return EDcDeserializePredicateResult::Pass;

	return ArrayProperty->HasMetaData(TEXT("DcExtraBase64"))
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}
```

[1]: https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/GameplayArchitecture/Metadata/ "Metadata Specifiers"