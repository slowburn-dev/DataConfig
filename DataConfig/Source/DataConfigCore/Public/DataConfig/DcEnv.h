#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

struct FDcReader;
struct FDcWriter;

struct DATACONFIGCORE_API FDcEnv
{
	TArray<FDcDiagnostic> Diagnostics;

	TSharedPtr<IDcDiagnosticConsumer> DiagConsumer;

	TArray<FDcReader*> ReaderStack;
	TArray<FDcWriter*> WriterStack;

	FDcDiagnostic& Diag(FDcErrorCode InErr);

	void FlushDiags();

	FORCEINLINE FDcDiagnostic& GetLastDiag() 
	{
		checkf(Diagnostics.Num(), TEXT("<empty diagnostics>"));
		return Diagnostics.Last();
	}

	~FDcEnv();
};

DATACONFIGCORE_API FDcEnv& DcEnv();
DATACONFIGCORE_API FDcEnv& DcPushEnv();
DATACONFIGCORE_API void DcPopEnv();

extern TBasicArray<FDcEnv> Envs;	// external linkage for debug watch window

template<typename T, TArray<T*> FDcEnv::*MemberPtr>
struct TScopedEnvMemberPtr
{
	TScopedEnvMemberPtr(T* InPtr)
	{
		Pointer = InPtr;
		(DcEnv().*MemberPtr).Push(Pointer);
	}

	~TScopedEnvMemberPtr()
	{
		check((DcEnv().*MemberPtr).Top() == Pointer);
		(DcEnv().*MemberPtr).Pop();
	}

	T* Pointer;
};

using FScopedStackedReader = TScopedEnvMemberPtr<FDcReader, &FDcEnv::ReaderStack>;
using FScopedStackedWriter = TScopedEnvMemberPtr<FDcWriter, &FDcEnv::WriterStack>;

struct DATACONFIGCORE_API FDcScopedEnv
{
	FDcScopedEnv();
	~FDcScopedEnv();
};

#define DC_DIAG(DiagNamespace, DiagID) (FDcErrorCode {DiagNamespace::Category, DiagNamespace::DiagID})

#define DC_FAIL(DiagNamespace, DiagID) (DcFail(FDcErrorCode{DiagNamespace::Category, DiagNamespace::DiagID}))

FORCEINLINE FDcDiagnostic& DcFail(FDcErrorCode InErr)
{
#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return DcEnv().Diag(InErr);
}

FORCEINLINE FDcDiagnostic& DcFail(uint16 InCategory, uint16 InCode)
{
#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return DcEnv().Diag({InCategory, InCode});
}

FDcResult DcFail();

FORCEINLINE FDcResult DcNoEntry() 
{
	checkNoEntry();
	return DcFail(1, 2);	// DcDCommon::Unreachable
}

//	global initializer and shutdown
enum class EDcInitializeAction
{
	Minimal,
	SetAsConsole,
};

DATACONFIGCORE_API void DcStartUp(EDcInitializeAction InAction = EDcInitializeAction::Minimal);
DATACONFIGCORE_API void DcShutDown();
DATACONFIGCORE_API bool DcIsInitialized();
