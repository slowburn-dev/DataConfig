#pragma once

#include "CoreMinimal.h"
#include "Containers/BasicArray.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

struct DATACONFIGCORE_API FDcEnv
{
	TArray<FDcDiagnostic> Diagnostics;

	TSharedPtr<IDcDiagnosticConsumer> DiagConsumer;

	FDcDiagnostic& Diag(FDcErrorCode InErr);

	void FlushDiags();

	FORCEINLINE FDcDiagnostic& GetLastDiag() { return Diagnostics.Last(); }

	~FDcEnv();
};

DATACONFIGCORE_API FDcEnv& DcEnv();
DATACONFIGCORE_API FDcEnv& DcPushEnv();
DATACONFIGCORE_API void DcPopEnv();

extern TBasicArray<FDcEnv> Envs;	// external linkage for debug watch window

struct DATACONFIGCORE_API FDcScopedEnv
{
	FDcScopedEnv();
	~FDcScopedEnv();
};

#define DC_DIAG(DiagNamespace, DiagID) (FDcErrorCode {DiagNamespace::Category, DiagNamespace::DiagID})

#define DC_FAIL(DiagNamespace, DiagID) (DcFail(FDcErrorCode{DiagNamespace::Category, DiagNamespace::DiagID}))

FORCEINLINE FDcDiagnostic& DcFail(FDcErrorCode InErr) {

#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return DcEnv().Diag(InErr);
}

FORCEINLINE FDcDiagnostic& DcFail(uint16 InCategory, uint16 InCode) {

#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return DcEnv().Diag({InCategory, InCode});
}

FORCEINLINE FDcResult DcFail() {
	return FDcResult{ FDcResult::EStatus::Error };
}

template<typename TThunk>
FORCEINLINE FDcResult DcExpect(bool CondToBeTrue, const TThunk& ErrFunc)
{
	if (CondToBeTrue)
		return DcOk();
	else
		return ErrFunc();
}

FDcResult DcExpect(bool CondToBeTrue);	// placeholder expect



