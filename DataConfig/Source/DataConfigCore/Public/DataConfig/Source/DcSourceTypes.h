#pragma once

#include <HAL/Platform.h>
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcSourceLocation
{
	uint32 Line;
	uint32 Column;
};

template<class CharType = TCHAR>
struct TDcSourceBuffer
{
	CharType* Buffer;
	int32 Num;

	FORCEINLINE CharType* End() 
	{
		return Buffer + Num;
	}
};

template<class CharType = TCHAR>
struct TDcSourceRef
{
	TDcSourceBuffer<CharType>* SourceBuffer = nullptr;

	int32 Begin;
	int32 Num;

	FString ToString();
};

#include "DataConfig/Source/DcSourceTypes.inl"

using FDcAnsiSourceBuffer = TDcSourceBuffer<ANSICHAR>;
using FDcSourceBuffer = TDcSourceBuffer<TCHAR>;

using FDcAnsiSourceRef = TDcSourceRef<ANSICHAR>;
using FDcSourceRef = TDcSourceRef<TCHAR>;



