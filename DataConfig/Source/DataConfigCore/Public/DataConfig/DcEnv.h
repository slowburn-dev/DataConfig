#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FEnv
{
	TArray<FDiagnostic> Diagnostics;

	TSharedPtr<IDiagnosticConsumer> DiagConsumer;

	FDiagnostic& Diag(FErrorCode InErr);

	void FlushDiags();

	~FEnv();
};

DATACONFIGCORE_API FEnv& Env();
DATACONFIGCORE_API FEnv& PushEnv();
DATACONFIGCORE_API void PopEnv();

struct DATACONFIGCORE_API FScopedEnv
{
	FScopedEnv();
	~FScopedEnv();
};

#define DIAG(DiagNamespace, DiagID) (FErrorCode {DiagNamespace::Category, DiagNamespace::DiagID})

FORCEINLINE FDiagnostic& Fail(FErrorCode InErr) {

#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return Env().Diag(InErr);
}

FORCEINLINE FDiagnostic& Fail(uint16 InCategory, uint16 InCode) {

#if DO_CHECK
	UE_DEBUG_BREAK();
#endif

	return Env().Diag({InCategory, InCode});
}

template<typename TThunk>
FORCEINLINE FResult Expect(bool CondToBeTrue, const TThunk& ErrFunc)
{
	if (CondToBeTrue)
		return Ok();
	else
		return ErrFunc();
}

FResult Expect(bool CondToBeTrue);	// placeholder expect

} // namespace DataConfig



