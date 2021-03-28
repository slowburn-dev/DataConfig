#pragma once

#include "DataConfig/Source/DcSourceTypes.h"

template<class CharType = TCHAR>
struct TDcCSourceUtils
{
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
			return 1;	// so that highlight can points to line end
		}
		else
		{
			checkNoEntry();
			return 0;
		}
	}

	static bool IsControl(const CharType& Char)
	{
		return (uint32)Char < 0x20 || Char == 0x7f;
	}

	static bool IsDigit(const CharType& Char)
	{
		return CChar::IsDigit(Char);
	}

};

namespace FDcSourceUtils
{
	FString FormatDiagnosticLine(const FString& InLine);


}	// namespace FDcSourceUtils




