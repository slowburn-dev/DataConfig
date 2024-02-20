# Changes

All notable changes to this project will be documented in this file.

## 1.5.0 - 2024-2-20

- **BREAKING** `EDcDataEntry::Nil` renamed to `EDcDataType::None`.
  - This is necessary to fix Mac builds.
- UE 5.4 support.
  - See [UE version upgrade 5.4](Advanced/UEUpgrades.md#ue54)
- **NEW** Optional support.
  - See [Optional](Extra/Optional.md)

## 1.4.3 - 2023-8-27

- **NEW** Extra samples.
  - [NDJSON](Extra/NDJSON.md)
  - [Root Object](Extra/RootObject.md)
  - [Nested Arrays](Extra/Nested.md)
- **CHANGE** small QoL changes:
  - Fix `FDcDiagnostic` enum argument captures.
  - Better `FDcJsonReader::FinishRead()` behavior.
  - Diagnostic message fixes.
  - Relax JSONReader a bit to allow root object/array.
  - Add `PredicateIsRootProperty` util to select root property.

## 1.4.2 - 2023-8-1

- UE 5.3 support.
  - See: [UE version upgrade 5.3](Advanced/UEUpgrades.md#ue53)


## 1.4.1 - 2023-2-15

- Fix compilation on `!WITH_EDITORONLY_DATA`.
  - See [No MetaData](Advanced/NoMetaData.md).
- UE 5.2 support.
  - See: [UE version upgrade 5.2](Advanced/UEUpgrades.md#ue52)

## 1.4.0 - 2022-11-17

Checkout blog post ["DataConfig 1.4 Released with UE 5.1 Support"](/blog/dataconfig-1-4/).

- **NEW** Support for UE 5.1.
  - See [UE version upgrade](Advanced/UEUpgrades.md).
- **NEW** Extra samples and docs.
  - [Advanced - Automation](Advanced/Automation.md)
  - [Extra - InstancedStruct](Extra/InstancedStruct.md)
  - [Configuring with `FPropertyConfig`](Formats/Property.md#configuring-with-fdcpropertyconfig)
  - [Writing Handlers - Coercion](Advanced/WritingHandlers.md#coercion)
- **CHANGE** Use separated `uplugin` for UE4 and UE5.
  - See [Integration](Integration.md#integrate-dataconfig-plugin)
- **FIX** Core changes and fixes:
  - Fail when `FPropertyWriter::WriteObjectReference()` takes nullptr.
    See test: `DataConfig.Core.Property.DiagNullObject`
  - Additional check for class mismatch for default object deserialize handlers.
    See test: `DataConfig.Core.Deserialize.DiagObjectClassMismatch`

## 1.3.0 - 2022-6-20

Checkout blog post ["DataConfig Core and JSON Asset 1.3 Release"](/blog/dataconfig-dcjsonasset-1-3/).

- **CHANGE** Property reader/writer improvement and fixes 
    - Allow reading `Array/Set/Map` and  native array like `int arr[5]` as root.<br>
      See: [Non Class/Struct Root](Advanced/NonClassStructRoot.md)
    - Expand property with `ArrayDim > 1` as array.<br>
      See: [Property](Formats/Property.md)
    - Performance improvement by caching `FScript/Array/SetHelper`
- **NEW** Extra samples:
    - [Dump Asset To Log](Extra/DumpAssetToLog.md)
    - [Deserialize From SQLite Query](Extra/SQLite.md)
    - [Inline Struct](Extra/InlineStruct.md)
- **FIX** Core changes and fixes:
    - Fix stale enum property fields serialization crash.
    - Fix `TSet/TMap` serialization crashes.
    - Fix soft object/class reference serialize to nil when it's not loaded.
    - Fix pipe property class/object handlers.
    - Fix `TObjectPtr<>` serialialzation and deserialization.
    - Fix `PeekReadDataPtr` on class property.
    - Removed `FScopedStackedReader/FScopedStackedWriter` usage.
    - Fix `HeuristicVerifyPointer` diagnostic.  
    - Fix `DC_TRY` shadowing variable `Ret`.
    - JSON now support non string keyed `TMap<>` as `[{ "$key": <foo>, "$value": <bar> }]`.<br>
      See: [JSON - Map](Formats/JSON.md#map)
    - Fix `DcPropertyUtils::DcIsSubObjectProperty()`<br>
      Now it only checks for `CPF_InstancedReference`.
    - Update screenshots to UE5.<br> Note that DataConfig [still supports from 4.25 and onwards](Integration.md).
    - Update [Property](Formats/Property.md) docs.

## 1.2.2 - 2022-4-5

- Support for [UE 5.0.0](https://docs.unrealengine.com/5.0/en-US/unreal-engine-5-0-release-notes/)
- Add `DebugGetRealPropertyValue` for double BP fields. 

## 1.2.1 - 2022-2-23

- Support for [UE5 Preview 1](https://www.unrealengine.com/en-US/blog/unreal-engine-5-is-now-available-in-preview).
- **FIX** Compile fixes for examples on UE 4.25.
- **FIX** UE 4.25/4.26 editor extra BP serde automation test fixes.
- **FIX** `FDcAnsiJsonWriter` writes non ascii char to `?` when string contains escaping characters.
    - `FDcJsonWriter` was unaffected. This only happens to the ansi char writer and only when input has escapes like `\t`.

## 1.2.0 - 2022-1-26

Checkout blog post ["Introducing DataConfig 1.2"](/blog/dataconfig-1-2/).

* **NEW** Serializer. Previously we only have deserializers. 
  * Serializer API mirrors deserializers.
  * Builtin serialization and deserialization handlers are all roundtrip-able.
  * `DcDiagnosticDeserialize -> DcDiagnosticSerDe` for sharing diagnostics.
  * `DcDeserializeUtils -> DcSerDeUtils` for sharing code.
* **NEW** MsgPack reader and writer.
  * Full spec implemented, minus the "Timestamp extension type".
  * Integrate and passes [kawanet/msgpack-test-suite](https://github.com/kawanet/msgpack-test-suite).
* **NEW** JSON writer.
  * With `WIDECHAR/ANSICHAR` specialization as JSON Reader.
  * Accept config to output pretty or condensed output.
* **NEW** Builtin metas.
  * `DcSkip`  - skip marked fields
  * `DcMsgPackBlob`  - marked TArray<>/Struct would be read as blob by MsgPack SerDe
* **CHANGE** Core type changes.
  * `FDcReader/FDcWriter` changes.
    * `FDcStruct/ClassStat` renamed to `FDcStruct/ClassAccess`.
    * `ReadStruct/ClassRoot()` renamed to `ReadStruct/ClassRootAccess`.
    * Add`ReadStruct/ClassRoot()` that takes no argument for common use cases.
    * `FDcReader::Coercion()` now returns a `FDcResult`
    * `[Read/Write]Soft[Object/Class]Reference` takes `FSoftObjectPtr`.
    * RTTI with `GetId()` and `CastById()`
  * `FDcSerializer/FDcDeserializer` changes.
    * `DcDiagnosticDeserialize` -> `DcDiagnosticSerDe` for sharing diagnostics.
    * `DcDeserializeUtils` -> `DcSerDeUtils` for sharing code.
    * Add handlers to read/write `Soft/Lazy` references as is, without loading the object.
    * Implicit call `Properties.Push()` before context `Prepare()`.
    * Removed `FDcScopedProperty` in favor of `DcDeserializeUtils::RecursiveDeserialize()` it's more concise. 
  * `FDcPropertyReader/FDcPropertyWriter` changes.
    * When reading class object keys any one with `$` will be ignored.
      - previously only allow `$type`, `$path`.
      - note that struct by default don't check for these. It's trivia to add the logic if you want to.
    * Add `FDcPropertyReader::PeekReadDataPtr` matches with `PeekProperty`
  * `FDcJsonReader` changes.
    * Remove object key length limit, which was 2048 and it's incorrect.
      - Though `FName` is capped at 1024, which is a Unreal Engine limit.
    * Fix `ReadName()` which previously would fail.
    * Fix quoted string parsing/escaping  in `ParseQuotedString`
  * Add `EDcDataEntry::Extension`.
  * Add `FDcPropertyDatum` template constructor to directly construct one from a `FSturct*`.
  * `FPrettyPrintWriter` now print blobs with hash, previously it's pointer value.
  * `DcAutomationUtils::SerializeIntoJson/DeserializeIntoJson` -> `SerializeInto/DeserializeFrom` as we're supporting other formats.
  * Add `HeuristicVerifyPointer` to check common magic invalid pointers.
* Misc fixes and QOL improvements.
  * Fixed linux build. Now the headless program target cross compiles and runs under wsl.
  * Serializer/Deserializer now also report diagnostics.
  * Better Json reader diagnostic formatting. Now it clamps long lines properly.
  * Add `DataConfigEditorExtra.DcCoreTestsCommandlet` as tests runner.
  * Add performance benchmark.
  * Restructured DataConfig book for topics on serializer and MsgPack.

## 1.1.1 - 2021-10-6

- Support for UE 4.27.
- Support for UE 5.

## 1.1.0 - 2021-4-24

- Integrate [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite). Now `DcJSONParser` pass most of them. Skipped tests are also documented.
- `FDcAnsiJsonReader` now detect and convert non ASCII UTF8 characters. Previously these characters are dropped.
- Headless test runner pass along parameters to tests, for example
  `DataConfigHeadless-Win64-Debug.exe Parsing -- n_array_just_minus`