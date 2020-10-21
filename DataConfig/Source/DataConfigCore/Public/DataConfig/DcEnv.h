#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDcEnv
{
	TArray<FDcDiagnostic> Diagnostics;

	TSharedPtr<IDcDiagnosticConsumer> DiagConsumer;

	FDcDiagnostic& Diag(FDcErrorCode InErr);

	void FlushDiags();

	~FDcEnv();
};

DATACONFIGCORE_API FDcEnv& DcEnv();
DATACONFIGCORE_API FDcEnv& DcPushEnv();
DATACONFIGCORE_API void DcPopEnv();

struct DATACONFIGCORE_API FDcScopedEnv
{
	FDcScopedEnv();
	~FDcScopedEnv();
};

#define DC_DIAG(DiagNamespace, DiagID) (FDcErrorCode {DiagNamespace::Category, DiagNamespace::DiagID})

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

template<typename TThunk>
FORCEINLINE FDcResult DcExpect(bool CondToBeTrue, const TThunk& ErrFunc)
{
	if (CondToBeTrue)
		return DcOk();
	else
		return ErrFunc();
}

FDcResult DcExpect(bool CondToBeTrue);	// placeholder expect

} // namespace DataConfig



