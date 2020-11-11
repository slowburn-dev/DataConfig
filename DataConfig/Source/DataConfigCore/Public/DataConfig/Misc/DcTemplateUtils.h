#pragma once

template<typename T>
struct TDcRestore
{
	TDcRestore(T& InRef, const T& NewValue)
		: Ref(InRef)
	{
		RestoreValue = Ref;
		Ref = NewValue;
	}

	TDcRestore(T& InRef)
		: TDcRestore(InRef, InRef)
	{}

	~TDcRestore()
	{
		Ref = RestoreValue;
	}

	T& Ref;
	T RestoreValue;
};

