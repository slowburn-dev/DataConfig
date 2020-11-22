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

FDcResult DcExpect(bool CondToBeTrue);	// placeholder expect

