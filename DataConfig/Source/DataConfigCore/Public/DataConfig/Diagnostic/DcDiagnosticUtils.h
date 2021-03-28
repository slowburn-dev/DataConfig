#pragma once

#include "DataConfig/DcEnv.h"

template<typename TThunk>
FORCEINLINE FDcResult DcExpect(bool CondToBeTrue, const TThunk& ErrFunc)
{
	if (CondToBeTrue)
		return DcOk();
	else
		return ErrFunc();
}

DATACONFIGCORE_API FDcResult DcExpect(bool CondToBeTrue);	// placeholder expect

DATACONFIGCORE_API FDcResult DcReadNextExpect(FDcReader& Reader, EDcDataEntry Expect);

struct DATACONFIGCORE_API FDcScopedDiagHandler
{
	using DiagHandlerType = TFunctionRef<void(FDcDiagnostic&)>; 
	FDcScopedDiagHandler(DiagHandlerType InHandler);
	~FDcScopedDiagHandler();

	int StartDiagCount;
	DiagHandlerType Handler;
};


namespace DcDiagnosticUtils
{

DATACONFIGCORE_API void AmendDiagnostic(FDcDiagnostic& Diag, FDcReader* Reader, FDcWriter* Writer);

DATACONFIGCORE_API FString StackWalkToString(int32 IgnoreCount);

} // namespace DcDiagnosticUtils



