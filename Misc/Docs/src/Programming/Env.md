# Env

DataConfig put most global state into a stack of `FDcEnv`:

```c++
// DataConfig/Source/DataConfigCore/Public/DataConfig/DcEnv.h
struct DATACONFIGCORE_API FDcEnv
{
    TArray<FDcDiagnostic> Diagnostics;

    TSharedPtr<IDcDiagnosticConsumer> DiagConsumer;

    TArray<FDcReader*> ReaderStack;
    TArray<FDcWriter*> WriterStack;

    bool bExpectFail = false;   // mute debug break

    FDcDiagnostic& Diag(FDcErrorCode InErr);

    void FlushDiags();

    FORCEINLINE FDcDiagnostic& GetLastDiag() 
    {
        checkf(Diagnostics.Num(), TEXT("<empty diagnostics>"));
        return Diagnostics.Last();
    }

    ~FDcEnv();
};
```
DataConfig needs explicit initializatioon before use. This is done through manually `DcStartUp()`. There's also a paired `DcShutdown()` that should be called when DataConfig isn't used anymore. Here's an example:

```c++
// DataConfig/Source/DataConfigEditorExtra/Private/DcEditorExtraModule.cpp
void FDcEditorExtraModule::StartupModule()
{
    UE_LOG(LogDataConfigCore, Log, TEXT("DcEditorExtraModule module starting up"));
    DcRegisterDiagnosticGroup(&DcDExtra::Details);
    DcRegisterDiagnosticGroup(&DcDEditorExtra::Details);

    DcStartUp(EDcInitializeAction::Minimal);
    //...
}

void FDcEditorExtraModule::ShutdownModule()
{
    DcShutDown();
    //...
}
```

The active `FDcEnv` is accesed by calling global function `DcEnv()`. Inside the Env:

- `Diagnostics`: all diagnostics are flushed into env.
- `DiagConsumer`: diagnostic handler, used to flush to log or `MessageLog` or maybe on screen.
- `ReaderStack/WriterStack`: used to pass along reader/writer down the callstack. See `FScopedStackedReader` uses for example.   
- ... and everything else.

You can use `DcPushEnv()` to create new env, and destroy old one by calling `DcPopEnv()`. At this moment it's mostly used to handle reentrant during serialization. See `FDcScopedEnv` uses for example.

