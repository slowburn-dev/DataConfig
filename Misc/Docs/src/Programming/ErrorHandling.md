# Error Handling

Proper error handling is crucial to implement robust serialization as it needs to deal with unknown user input. DataConfig also provide diagnostic to help users quickly pin down common errors like typo or missing colons in JSON. Here's an example:

```
# DataConfig Error: Enum name not found in enum type: EDcTestExampleEnum, Actual: 'Far'
- [JsonReader] --> <in-memory>4:25
   2 |    {
   3 |        "StrField" : "Lorem ipsum dolor sit amet",
   4 |        "EnumField" : "Far",
     |                           ^
   5 |        "Colors" : [
   6 |            "#FF0000FF", "#00FF00FF", "#0000FFFF"
- [PropertyWriter] Writing property: (FDcTestExampleStruct)$root.(EEDcTestExampleEnum)EnumField
```

Internally DataConfig is applying a consistent error handling strategy across all API. User code are expected to follow along.

## Returning `FDcResult`

The gist is that if a method can fail, return a `FDcResult`, which is a simple struct:

```c++
// DataConfig/DataConfig/Source/DataConfigCore/Public/DataConfig/DcTypes.h
struct DATACONFIGCORE_API DC_NODISCARD FDcResult
{
    enum class EStatus : uint8
    {
        Ok,
        Error
    };

    EStatus Status;

    FORCEINLINE bool Ok() const
    {
        return Status == EStatus::Ok;
    }
};

//  See FDcReader's methods as example
// DataConfig/Source/DataConfigCore/Public/DataConfig/Reader/DcReader.h
struct DATACONFIGCORE_API FDcReader
{
    //...
    virtual FDcResult ReadBool(bool* OutPtr);
    virtual FDcResult ReadName(FName* OutPtr);
    virtual FDcResult ReadString(FString* OutPtr);
    virtual FDcResult ReadText(FText* OutPtr);
    virtual FDcResult ReadEnum(FDcEnumData* OutPtr);
    //...
};
```

Then use `DC_TRY` to call these kinds of functions. The macro itself does early return when result is not ok:

```c++
// DataConfig/DataConfig/Source/DataConfigCore/Public/DataConfig/DcTypes.h
#define DC_TRY(expr)                        \
    do {                                    \
        ::FDcResult Ret = (expr);           \
        if (!Ret.Ok()) {                    \
            return Ret;                     \
        }                                   \
    } while (0)

//  Example of calling methods returning `FDcResult`
// DataConfig/Source/DataConfigExtra/Private/DataConfig/Extra/Deserialize/DcDeserializeColor.cpp
template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriterAPI>(const FColor& Color, FDcDeserializeContext& Ctx)
{
    DC_TRY(Ctx.Writer->WriteStructRoot(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

    DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
    DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

    DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
    DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

    DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
    DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

    DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
    DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

    DC_TRY(Ctx.Writer->WriteStructEnd(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

    return DcOk();
}
```
This pattern is similar to [Outcome](https://ned14.github.io/outcome/) and [std::expected](https://wg21.link/p0323) except we give up using the return value. Return values should be passed through reference or pointers in function arguments.

## Diagnostics

When implementing a method that returns `FDcResult` you have 2 options:

- Return `DcOk()` on succeed.
- Return `DC_FAIL(<Catetory>, <ErrId>)` on error.

Examples:

```c++
// DataConfig/Source/DataConfigTests/Private/DcTestBlurb.cpp
FDcResult Succeed() {
    // succeed
    return DcOk();
}

FDcResult Fail() {
    // fail !
    return DC_FAIL(DcDCommon, Unexpected1) << "My Custom Message";
}
```

In the examples above `DcDCommon` and `Unexpected1` are called __error category__ and __error id__ respectively. `DcDCommon` is a built-in error category:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/Diagnostic/DcDiagnosticCommon.h
namespace DcDCommon
{
static const uint16 Category = 0x1;

enum Type : uint16
{
    //...
    Unexpected1,
};

} // namespace DcDCommon

// DataConfig/Source/DataConfigCore/Private/DataConfig/Diagnostic/DcDiagnosticCommon.cpp
namespace DcDCommon
{
static FDcDiagnosticDetail _CommonDetails[] = {
    // ...
    { Unexpected1, TEXT("Unexpected: '{0}'") },
};
```

Note that we can pipe argument into the diagnostic. The diagnostic reported by invoking `Fail()` would be like:

```
* # DataConfig Error: Unexpected: 'My Custom Message'
```

## Conclusion

DataConfig uses `FDcResult`, `DC_TRY`, `DC_FAIL` for error handling. It's lightweight and relatively easy to grasp. There's still some limitations in this regard:

- `FDcResult` occupied the return position making passing value to parent a bit cumbersome.
- For now we always stop as the first error. There's no plan to support error recovery.

Some closing notes:

- Reported diagnostics get queued. You'll need to call `FDcEnv::FlushDiags()` to flush them to consumers.
- See `DcDiagnosticExtra.h/cpp` for how to register user category.
- See `DcEditorExtraModule.cpp - FDcMessageLogDiagnosticConsumer` for custom diagnostic handler and formatting.