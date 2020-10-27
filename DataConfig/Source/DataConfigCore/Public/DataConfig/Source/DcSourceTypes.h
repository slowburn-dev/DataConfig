#pragma once

#include "HAL/Platform.h"
#include "Misc/CString.h"
#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcSourceLocation
{
	uint32 Line;
	uint32 Column;
};


template<class CharType = TCHAR>
struct TDcSourceBuffer
{
	using TCharType = CharType;

	const CharType* Buffer = nullptr;
	int32 Num = 0;

	TDcSourceBuffer() = default;
	TDcSourceBuffer(const TDcSourceBuffer&) = default;

	TDcSourceBuffer(const CharType* Ptr)
	{
		Buffer = Ptr;
		Num = TCString<CharType>::Strlen(Ptr);
	}

	FORCEINLINE CharType Get(int32 Ix) const
	{
		check(Ix >= 0 && Ix < Num);
		return Buffer[Ix];
	}
};

template<class CharType = TCHAR>
struct TDcSourceRef
{
	using TCharType = CharType;

	const TDcSourceBuffer<CharType>* Buffer = nullptr;

	int32 Begin;
	int32 Num;

	FString ToString() const;

	bool IsValid() const;
	void Reset();
};

#include "DataConfig/Source/DcSourceTypes.inl"

using FDcAnsiSourceBuffer = TDcSourceBuffer<ANSICHAR>;
using FDcSourceBuffer = TDcSourceBuffer<TCHAR>;

using FDcAnsiSourceRef = TDcSourceRef<ANSICHAR>;
using FDcSourceRef = TDcSourceRef<TCHAR>;



