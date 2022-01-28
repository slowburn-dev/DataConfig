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
struct TDcSourceView
{
	using TCharType = CharType;

	const CharType* Buffer = nullptr;
	int32 Num = 0;

	TDcSourceView() = default;
	TDcSourceView(const TDcSourceView&) = default;

	TDcSourceView(const CharType* InPtr)
	{
		Buffer = InPtr;
		Num = TCString<CharType>::Strlen(InPtr);
	}

	TDcSourceView(const CharType* InPtr, int32 InNum)
	{
		Buffer = InPtr;
		Num = InNum;
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

	const TDcSourceView<CharType>* Buffer = nullptr;

	int32 Begin;
	int32 Num;

	FString CharsToString() const;

	bool IsValid() const;
	void Reset();
	const CharType* GetBeginPtr() const;
};

#include "DataConfig/Source/DcSourceTypes.inl"

using FDcAnsiSourceBuffer = TDcSourceView<ANSICHAR>;
using FDcSourceBuffer = TDcSourceView<TCHAR>;

using FDcAnsiSourceRef = TDcSourceRef<ANSICHAR>;
using FDcSourceRef = TDcSourceRef<TCHAR>;



