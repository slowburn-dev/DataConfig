#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Misc/DcDiagnostic.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FEnv
{
	TArray<FDiagnostic> Diagnostics;

	FDiagnostic& Diag(FErrorCode InErr);
};

DATACONFIGCORE_API FEnv& Env();
DATACONFIGCORE_API FEnv& PushEnv();
DATACONFIGCORE_API void PopEnv();

struct DATACONFIGCORE_API FScopedEnv
{
	FScopedEnv();
	~FScopedEnv();
};

FORCEINLINE FResult Fail(FErrorCode InErr) {
	return Env().Diag(InErr);
}
FORCEINLINE FResult Fail(uint16 InCategory, uint16 InCode) {
	return Env().Diag({InCategory, InCode});
}

} // namespace DataConfig



