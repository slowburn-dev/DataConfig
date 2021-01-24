# Design

This page documents the overall design, goals and reasoning around DataConfig.

## Rationale

At the time we started this project we're looking for a JSON parser that:

- Supports a relaxed JSON spec, i.e. comment and trailing comma.
- Supports custom deserialization logic, i.e. deserialize `FColor` from `#RRGGBBAA` string.
- Supports UE4 instanced sub objects and polymorphism during deserialization.

Eventually we implemented all these in DataConfig. We tried not limit this to be a JSON parser but deliver as a set of helpful tools for reading-from and writing-to the property system. 

## Manifests

- Deliver as a quality C++ source library.
  
  DataConfig should shipped with no UI nor tooling code. Users are expected to integrate only `DataConfigCore` as a source module. We're intentionally limiting the scope of DataConfig to be a "C++ Library". Our users should be proficient UE4 C++ programmers.
  
  - DataConfig should ship with good test and documentation coverage.
    - DataConfig follows idiomatic UE4 C++ conventions and has no external dependency.
    - `DataConfigCore` only depends on `Core` and `CoreUObject` and can be used in standalone `Program` targets.
    - It comes with `DataConfigHeadless` program target that runs automation tests as a console app.
    - DataConfig API are `UObject` free and stack friendly.
    - Built-in features serve as examples and sensible defaults. Users are expected to write on their own `Reader/Writer/Handlers`.

- Favor idiomatic and user friendly over runtime performance.

  We expect users to use DataConfig in a offline, editor only scenario. In this use case we favor some other aspects over runtime performance:

    - Follow [UE4 coding conventions][2] and keep core dependency to only `Core` and `CoreUObject`.

    - When processing invalid data and invalid API usage DataConfig should not crash. It should fail explicitly with detailed context and diagnostics.

[1]:https://www.unrealengine.com/en-US/blog/unreal-property-system-reflection "Unreal Property System (Reflection)"
[2]:https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/index.html "UE4 Coding Standard"
