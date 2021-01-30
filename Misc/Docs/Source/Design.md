# Design

This page documents the overall design, goals and reasoning around DataConfig.

## Rationale

At the time we started this project we're looking for a JSON parser that:

- Supports a relaxed JSON spec, i.e. comment and trailing comma.
- Supports custom deserialization logic, i.e. deserialize `FColor` from `#RRGGBBAA` string.
- Supports UE4 instanced sub objects and polymorphism.

Eventually we implemented all these in DataConfig. We also tried not limit this to be a JSON parser but deliver as a set of helpful tools for reading-from and writing-to the property system. 

## Manifests

- Deliver as a quality C++ source library.
  
  DataConfig should ship with no UI nor tooling code. Users are expected to integrate only `DataConfigCore` as a source module. We're intentionally limiting the scope of DataConfig to be a "C++ Library". Our users should be proficient UE4 C++ programmers.
  
  - DataConfig should ship with good test and documentation coverage.
  - DataConfig follows idiomatic UE4 C++ conventions and has no external dependency.
  - `DataConfigCore` only depends on `Core` and `CoreUObject` and can be used in standalone `Program` targets.
  - DataConfig API are `UObject` free and stack friendly.
  - Built-in features serve as examples and sensible defaults. Users are expected to write on their own `Reader/Writer/Handlers`.
  
- Favor idiomatic, user friendly, small code size over runtime performance.

  We expect users to use DataConfig in a offline, editor only scenario. In this use case we favor some other aspects over runtime performance:

    - Follow [UE4 coding conventions][2] and keep core dependency to only `Core` and `CoreUObject`.
  - When processing invalid data and invalid API usage DataConfig should not crash. It should fail explicitly with detailed context and diagnostics.
  - DataConfig try not to don't expose template API. `TDcJsonReader` is explicit instantiated with its definition in private files.
  
- Works with whatever property system supports.

  The idea is that DataConfig supports everything that can be marked with `UCLASS/USTRUCT/UPROPERTY/UENUM` macros, which covers the full data model of the property system.

  Fields like weak object reference and delegates doesn't make much sense to be serialized into textual format. But turns out supporting the full data model makes DataConfig suitable to some other tasks like debug dump and in-memory data wrangling.

  This also means that DataConfig only focus on reading from and writing into C++ data structures. For example we don't have a DOM or object like API for JSON at all. The only use case DataConfig supports is to deserialize from JSON into native C++ objects.

[1]:https://www.unrealengine.com/en-US/blog/unreal-property-system-reflection "Unreal Property System (Reflection)"
[2]:https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/index.html "UE4 Coding Standard"
