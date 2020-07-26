#pragma once

#include "DataConfig/Misc/DcDataVariant.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDiagnostic
{
	uint32 ID;
	TArray<FDataVariant> Args;

	FDiagnostic(uint32 InID) : ID(InID)
	{}
};

template<typename T>
FORCEINLINE FDiagnostic& operator<<(FDiagnostic& Diag, T&& InValue)
{
	Diag.Args.Emplace(Forward<T>(InValue));
	return Diag;
}


} // namespace DataConfig



