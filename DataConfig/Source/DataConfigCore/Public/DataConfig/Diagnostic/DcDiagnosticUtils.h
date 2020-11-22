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

