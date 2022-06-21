#pragma once

#include "DataConfig/Source/DcSourceTypes.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/StringBuilder.h"

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25

namespace DcSourceUtilsDetails
{
template <typename CharType> struct TSBTypeSelector;
template <> struct TSBTypeSelector<ANSICHAR> { using Type = TAnsiStringBuilder<1024>; };
template <> struct TSBTypeSelector<WIDECHAR> { using Type = TStringBuilder<1024>; };
} // namespace DcSourceUtilsDetails

#endif

template<class CharType = TCHAR>
struct TDcCSourceUtils
{
#if ENGINE_MAJOR_VERSION == 5
	using StringBuilder = TStringBuilderWithBuffer<CharType, 1024>;
#else
	#if ENGINE_MINOR_VERSION >= 26
	using StringBuilder = TStringBuilderWithBuffer<CharType, 1024>;
	#else
	using StringBuilder = typename DcSourceUtilsDetails::TSBTypeSelector<CharType>::Type;
	#endif
#endif

	using SourceRef = TDcSourceRef<CharType>;
	using CChar = TChar<CharType>;

	static bool IsLineBreak(const CharType& Char)
	{
		return Char == CharType('\n');
	}

	static bool IsWhitespace(const CharType& Char)
	{
		return Char == CharType(' ')
			|| Char == CharType('\t')
			|| Char == CharType('\n')
			|| Char == CharType('\r');
	}

	static int GetWhitespaceSize(const CharType& Char)
	{
		if (Char == CharType(' '))
		{
			return 1;
		}
		else if (Char == CharType('\t'))
		{
			return 4;
		}
		else if (Char == CharType('\r'))
		{
			return 0;
		}
		else if (Char == CharType('\n'))
		{
			return 0;
		}
		else
		{
			checkNoEntry();
			return 0;
		}
	}

	static bool IsControl(const CharType& Char)
	{
		return (uint32)Char <= 0x1F ||
			(Char >= 0x7f && Char <= 0xa0);
	}

	static bool IsDigit(const CharType& Char)
	{
		return (unsigned)Char - CharType('0') < 10;
	}

	static bool IsHexDigit(const CharType& Char)
	{
		return IsDigit(Char)
			|| ((unsigned)Char|32) - CharType('a') < 6;
	}

	static bool IsOneToNine(const CharType& Char)
	{
		return Char != CharType('0') && IsDigit(Char);
	}

	static bool IsAscii(const CharType& Char)
	{
		return Char >= 0 && Char <= 0x7f;
	}
};





