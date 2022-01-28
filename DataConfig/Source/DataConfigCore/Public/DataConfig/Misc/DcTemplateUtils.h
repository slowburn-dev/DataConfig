#pragma once

#include "DataConfig/DcTypes.h"

//	shorthand for optional read to out pointer
template<typename T1, typename T2>
FORCEINLINE static void ReadOut(T1*& OutPtr, T2&& Value)
{
	if (OutPtr)
		*OutPtr = Forward<T2>(Value);
}

template<typename T1, typename T2>
FORCEINLINE static FDcResult ReadOutOk(T1*& OutPtr, T2&& Value)
{
	if (OutPtr)
		*OutPtr = Forward<T2>(Value);

	return DcOk();
}

template<typename T>
struct TDcStoreThenReset
{
	TDcStoreThenReset(T& InRef, const T& NewValue)
		: Ref(InRef)
	{
		RestoreValue = Ref;
		Ref = NewValue;
	}

	TDcStoreThenReset(T& InRef)
		: Ref(InRef)
	{
		RestoreValue = Ref;
	}

	~TDcStoreThenReset()
	{
		Ref = RestoreValue;
	}

	T& Ref;
	T RestoreValue;
};

template <typename T>
constexpr bool DcIsPowerOf2(T Value)
{
	return (Value & (Value-1)) == 0;
}

