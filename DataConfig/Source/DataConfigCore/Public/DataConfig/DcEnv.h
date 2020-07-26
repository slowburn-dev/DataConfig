#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Misc/DcDiagnostic.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FEnv
{
	TArray<FDiagnostic> Diagnostics;

	FDiagnostic& Diag(uint32 DiagID);
};

DATACONFIGCORE_API FEnv& Env();
DATACONFIGCORE_API FEnv& PushEnv();
DATACONFIGCORE_API void PopEnv();

struct DATACONFIGCORE_API FScopedEnv
{
	FScopedEnv();
	~FScopedEnv();
};


} // namespace DataConfig



