#pragma once

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

