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

FORCEINLINE FDiagnostic& Fail(FErrorCode InErr) {
	return Env().Diag(InErr);
}

FORCEINLINE FDiagnostic& Fail(uint16 InCategory, uint16 InCode) {
	return Env().Diag({InCategory, InCode});
}

} // namespace DataConfig



