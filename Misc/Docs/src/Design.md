# Design

This page documents the overall design, goals and reasoning around DataConfig.

## Rationale

At the time we started this project we're looking for a JSON parser that:

- Supports a relaxed JSON spec, i.e. comment and trailing comma.
- Supports custom deserialization logic, i.e. deserialize `FColor` from `#RRGGBBAA` string.
- Supports UE4 instanced sub objects and polymorphism during deserialization.

Eventually we implemented all these in DataConfig. Additionally we tried not limit this to be a JSON parser but deliver as a set of helpful tools for reading-from and writing-to the property system. 

## Manifests

- Deliver as a quality C++ source library.
  
  DataConfig should shipped with no UI nor tooling code. Users are expected to integrate only `DataConfigCore` as a source module.
  We're intentionally limitting the scope to "C++ Library" and aiming to meet the expectation of proficient UE4 C++ programmers.

    - DataConfig should ship with good test and documentation coverage.
    - `DataConfigCore` only depends on `Core` and `CoreUObject` and can be used in standalone `Program` targets.
    - There's `DataConfigHeadless` program target that runs automation tests as a console app.
    - DataConfig classes are `UObject` free, making the API stack friendly.

- Prefer idiomatic and user friendly over runtime performance.

  We expect users to use DataConfig in a offline, editor only scenario. In this use case we favor some other aspects over runtime performance:

    - Follow [UE4 coding conventions][2] and keep core dependency to only `Core` and `CoreUObject`.

    - Expect data we read in are always manually typed in by human. Thus we should provide helpful diagnostics that can help users to track down mistakes fast.

[1]:https://www.unrealengine.com/en-US/blog/unreal-property-system-reflection "Unreal Property System (Reflection)"
[2]:https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/index.html "UE4 Coding Standard"
