# Design

This page documents the overall design, goals and reasoning around DataConfig.

## Rationale

At the time we started this project we were looking for a JSON parser that:

- Supports a relaxed JSON spec, i.e. comment and trailing comma.
- Supports custom deserialization logic, i.e. deserializes `FColor` from `#RRGGBBAA`.
- Supports UE4 instanced sub objects and polymorphism.

Ultimately we implemented all these in DataConfig. We also tried not to limit this to be a JSON parser but to provide a set of tools for reading-from and writing-to the property system. 

If you are an Unreal Engine C++ developers that:

- Looking for alternative JSON reader/writer.
- Looking for MsgPack serializer.
- Looking for a textual configuration format.
- Thinking of implementing custom textual/binary format.
- Write code that deals with `FProperty` on a daily bases.

You should try DataConfig and it's highly likely DataConfig will fit into your solution.

## Manifesto

- Deliver as a quality C++ source library.
  
  DataConfig should ship with no UI nor tooling code. Users are expected to integrate only `DataConfigCore` as a source module. We intentionally limit the scope of DataConfig to a "C++ Library". Our users should be proficient UE4 C++ programmers.
  
  - DataConfig should ship with good testing and documentation coverage.
  - DataConfig follows idiomatic UE4 C++ conventions and has no external dependencies.
  - `DataConfigCore` depends only on `Core` and `CoreUObject` and can be used in standalone `Program` targets.
  - DataConfig API are `UObject` free and stack friendly.
  - Built-in features serve as examples and sensible defaults. Users are expected to write on their own `Reader/Writer/Handlers`.
  
- Runtime performance is *not* our top priority.

  We expect users to use DataConfig in an offline, editor only scenario. In this use case we favor some other aspects over runtime performance:

  - Idiomatic. We follow [UE4 c++ coding conventions][2] and keep core dependency to only `Core` and `CoreUObject`.
  - Friendly. When processing invalid data and invalid API usage DataConfig should not crash. It should fail explicitly with detailed context and diagnostics.
  - Small code size / fast compile time. DataConfig tries not to expose template API. `TDcJsonReader` is explicit instantiated with its definition in private files.
  - Light memory footprint. Our JSON parser does stream parsing and would *not* construct the loaded JSON document in memory at all.

- Works with whatever property system supports.

  The idea is that DataConfig supports everything that can be tagged with `UCLASS/USTRUCT/UPROPERTY/UENUM` macros, which covers the full data model of the property system.

  Fields such as weak object reference and delegates doesn't make much sense to be serialized into textual format. However it turns out supporting the full data model makes it suitable to some other tasks like debug dump and in-memory data wrangling.

  This also means that DataConfig focuses only on reading from and writing into C++ data structures. For example we do not have a DOM or object like API for JSON at all. The only use case DataConfig supports is to deserialize from JSON into native C++ objects.

## Acknowledgement

- References [serde.rs][3] on API and the `SerDe` acronym.
- References [FullSerializer][8] and [OdinSerializer][9] on API.
- JSON parser/writer implementation and test cases references [JSON for Modern C++ ][4] and [RapidJson][5].
- Integrated [nst/JSONTestSuite][6].
- Integrated [kawanet/msgpack-test-suite][7].

[1]:https://www.unrealengine.com/en-US/blog/unreal-property-system-reflection "Unreal Property System (Reflection)"
[2]:https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/index.html "UE4 Coding Standard"
[3]:https://serde.rs/ "SerDe"
[4]:https://json.nlohmann.me "JSON for Modern C++"
[5]:https://rapidjson.org/ "RapidJSON"
[6]:https://github.com/nst/JSONTestSuite "JSON Parsing Test Suite"
[7]:https://github.com/kawanet/msgpack-test-suite "a dataset for testing msgpack library"
[8]:https://github.com/jacobdufault/fullserializer "jacobdufault/fullserializer"
[9]:https://github.com/TeamSirenix/odin-serializer "TeamSirenix/odin-serializer"
