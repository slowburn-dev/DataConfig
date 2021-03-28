#include "DataConfig/Source/DcSourceUtils.h"

FString FDcSourceUtils::FormatDiagnosticLine(const FString& InLine)
{
	using DcSourceUtils = TDcCSourceUtils<TCHAR>;

	FString OutLine;
	for (int Ix = 0; Ix < InLine.Len(); Ix++)
	{
		TCHAR Cur = InLine[Ix];
		if (DcSourceUtils::IsWhitespace(Cur))
		{
			int Size = DcSourceUtils::GetWhitespaceSize(Cur);
			for (int Jx = 0; Jx < Size; Jx++)
				OutLine.AppendChar(TCHAR(' '));
		}
		else
		{
			OutLine.AppendChar(Cur);
		}
	}

	return OutLine;
}
