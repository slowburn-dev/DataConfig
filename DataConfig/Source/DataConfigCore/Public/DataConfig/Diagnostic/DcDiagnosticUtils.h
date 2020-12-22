#pragma once

#include "Dataconfig/DcEnv.h"

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

namespace DcDiagnosticUtils
{

DATACONFIGCORE_API void AmendDiagnostic(FDcDiagnostic& Diag, FDcReader* Reader, FDcWriter* Writer);

DATACONFIGCORE_API FString StackWalkToString(int32 IgnoreCount);

} // namespace DcDiagnosticUtils



