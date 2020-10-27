#pragma once

#include "CoreMinimal.h"
#include "DataConfig/Source/DcSourceTypes.h"

template<class CharType = TCHAR>
struct TDcCSourceUtils
{
	using SourceRef = TDcSourceRef<CharType>;

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
};

namespace FDcSourceUtils
{
	FString FormatDiagnosticLine(const FString& InLine);


}	// namespace FDcSourceUtils



