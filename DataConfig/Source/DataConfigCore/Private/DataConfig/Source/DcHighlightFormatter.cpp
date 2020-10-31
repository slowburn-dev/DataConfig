#include "DataConfig/Source/DcHighlightFormatter.h"

template<class CharType>
FString FHightlightFormatter<CharType>::FormatHighlight(const SourceRef& SpanRef, const FDcSourceLocation& Loc)
{
	LineHighlight = FindLine(SpanRef);
	check(LineHighlight.IsValid());

	{
		SourceRef LineBefore = LineHighlight;
		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			LineBefore.Begin = LineBefore.Begin - 2;    // -1 is \n
			LineBefore.Num = 1;
			if (!LineBefore.IsValid())
				break;
			int Cur = _LINE_CONTEXT - Ix - 1;
			LinesBefore[Cur] = FindLine(LineBefore);
			LineBefore = LinesBefore[Cur];
		}
	}

	{
		SourceRef LineAfter = LineHighlight;
		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			LineAfter.Begin = LineAfter.Begin + LineAfter.Num;
			LineAfter.Num = 1;
			if (!LineAfter.IsValid())
				break;
			LinesAfter[Ix] = FindLine(LineAfter);
			LineAfter = LinesAfter[Ix];
		}
	}

	{
		TArray<FString, TFixedAllocator<_LINE_CONTEXT * 2 + 2>> Reports;

		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			if (!LinesBefore[Ix].IsValid())
				continue;

			FString LineStr = FDcSourceUtils::FormatDiagnosticLine(LinesBefore[Ix].ToString());
			int CurLine = Loc.Line - _LINE_CONTEXT + Ix;
			Reports.Add(FString::Printf(TEXT("%4d |%s"), CurLine, *LineStr));
		}

		{
			SourceRef PrefixRef = LineHighlight;
			PrefixRef.Num = SpanRef.Begin - LineHighlight.Begin;
			check(PrefixRef.IsValid());

			FString LineStr = FDcSourceUtils::FormatDiagnosticLine(LineHighlight.ToString());
			FString PrefixStr = FDcSourceUtils::FormatDiagnosticLine(PrefixRef.ToString());
			FString SpanStr = FDcSourceUtils::FormatDiagnosticLine(SpanRef.ToString());

			Reports.Add(FString::Printf(TEXT("%4d |%s"), Loc.Line, *LineStr));
			Reports.Add(FString::Printf(TEXT("     |%s%s"),
				*FString::ChrN(PrefixStr.Len(), TCHAR(' ')),
				*FString::ChrN(SpanStr.Len(), TCHAR('^'))));
		}

		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			if (!LinesAfter[Ix].IsValid())
				continue;

			FString LineStr = FDcSourceUtils::FormatDiagnosticLine(LinesAfter[Ix].ToString());
			int CurLine = Loc.Line + Ix + 1;
			Reports.Add(FString::Printf(TEXT("%4d |%s"), CurLine, *LineStr));
		}

		return FString::Join(Reports, TEXT("\n"));
	}
}

template<class CharType>
TDcSourceRef<CharType> FHightlightFormatter<CharType>::FindLine(const TDcSourceRef<CharType>& SpanRef)
{
	check(SpanRef.IsValid());

	const SourceBuf* Buf = SpanRef.Buffer;
	int32 CurHead = SpanRef.Begin - 1;
	while (CurHead >= 0)
	{
		if (SourceUtils::IsLineBreak(Buf->Get(CurHead)))
		{
			++CurHead;
			break;
		}
		--CurHead;
	}

	if (CurHead < 0)
		CurHead = 0;


	int32 CurTail = SpanRef.Begin;
	while (CurTail < Buf->Num)
	{
		if (SourceUtils::IsLineBreak(Buf->Get(CurTail++)))
			break;
	}

	return SourceRef{ Buf, CurHead, CurTail - CurHead };
}

template struct FHightlightFormatter<ANSICHAR>;
template struct FHightlightFormatter<WIDECHAR>;
