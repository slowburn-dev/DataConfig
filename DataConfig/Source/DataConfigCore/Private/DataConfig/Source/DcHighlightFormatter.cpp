#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

template<class CharType>
FString THightlightFormatter<CharType>::FormatHighlight(SourceRef SpanRef, int Line, FString* ReportFirstLine)
{
	if (!SpanRef.IsValid())
		return TEXT("<internal error source invalid>");

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

	if (Line == -1)
		Line = FindLineNumber(SpanRef);

	int LineOffset;
    {
        if (SpanRef.Num >= _LINE_MAX_LENGH)
            SpanRef.Num = _LINE_MAX_LENGH;

        //  calculate a line window 
        int Slack = _LINE_MAX_LENGH - SpanRef.Num;
        int SlackBefore = Slack / 2;
		int SlackAfter = Slack - SlackBefore;

		int SlackOff = FMath::Max(0, (SpanRef.Begin - LineHighlight.Begin + SpanRef.Num + SlackAfter) - LineHighlight.Num);
		SlackBefore += SlackOff;

        LineOffset = SpanRef.Begin - LineHighlight.Begin;
        LineOffset -= SlackBefore;
        if (LineOffset < 0)
            LineOffset = 0;
    }

	auto _ClampLine = [LineOffset](SourceRef Line)
	{
		Line.Begin += LineOffset;
		Line.Num = FMath::Clamp(Line.Num - LineOffset, 0, _LINE_MAX_LENGH);
		return Line;
	};

	auto _HasSuffix = [LineOffset](const SourceRef& Line)
	{
		return (Line.Num - LineOffset) > _LINE_MAX_LENGH;
	};

	{
		TArray<FString, TFixedAllocator<_LINE_CONTEXT * 2 + 2>> Reports;
		if (ReportFirstLine != nullptr)
			Reports.Add(*ReportFirstLine);

		const TCHAR* Prefix = LineOffset > 0 ? TEXT("...") : TEXT("");
		const TCHAR* PrefixWS = LineOffset > 0 ? TEXT("   ") : TEXT("");
		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			if (!LinesBefore[Ix].IsValid())
				continue;

			FString LineStr = FormatDiagnosticLine(_ClampLine(LinesBefore[Ix]).CharsToString());
			int CurLine = Line - _LINE_CONTEXT + Ix;
			const TCHAR* Suffix = _HasSuffix(LinesBefore[Ix]) ? TEXT("...") : TEXT("");
			Reports.Add(FString::Printf(TEXT("%4d |%s%s%s"), CurLine, Prefix, *LineStr, Suffix));
		}

		{
			SourceRef PrefixRef = LineHighlight;
			PrefixRef.Num = SpanRef.Begin - LineHighlight.Begin;
			check(PrefixRef.IsValid());

			FString LineStr = FormatDiagnosticLine(_ClampLine(LineHighlight).CharsToString());
			FString PrefixStr = FormatDiagnosticLine(PrefixRef.CharsToString());
			FString SpanStr = FormatDiagnosticLine(SpanRef.CharsToString());

			const TCHAR* Suffix = _HasSuffix(LineHighlight) ? TEXT("...") : TEXT("");
			Reports.Add(FString::Printf(TEXT("%4d |%s%s%s"), Line, Prefix, *LineStr, Suffix));
			Reports.Add(FString::Printf(TEXT("     |%s%s%s"),
				PrefixWS,
				*FString::ChrN(PrefixStr.Len() - LineOffset, TCHAR(' ')),
				*FString::ChrN(FMath::Max(SpanStr.Len(), 1), TCHAR('^'))));
		}

		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			if (!LinesAfter[Ix].IsValid())
				continue;

			FString LineStr = FormatDiagnosticLine(_ClampLine(LinesAfter[Ix]).CharsToString());
			uint32 CurLine = Line + Ix + 1;
			const TCHAR* Suffix = _HasSuffix(LinesAfter[Ix]) ? TEXT("...") : TEXT("");
			Reports.Add(FString::Printf(TEXT("%4d |%s%s%s"), CurLine, Prefix, *LineStr, Suffix));
		}

		return FString::Join(Reports, TEXT("\n"));
	}
}

template <class CharType>
int THightlightFormatter<CharType>::FindLineNumber(const SourceRef& SpanRef)
{
	check(SpanRef.IsValid());

	const SourceBuf* Buf = SpanRef.Buffer;
	int Line = 0;
	int CurIx = SpanRef.Begin;

	while (--CurIx > 0)
	{
		if (SourceUtils::IsLineBreak(Buf->Get(CurIx)))
			++Line;
	}

	return Line + 1;	// line number start from 1
}

template <class CharType>
FString THightlightFormatter<CharType>::FormatDiagnosticLine(const FString& InLine)
{
	using DcSourceUtils = TDcCSourceUtils<TCHAR>;
	FString OutLine;
	int Len = InLine.Len();;

	for (int Ix = 0; Ix < Len; Ix++)
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

template<class CharType>
TDcSourceRef<CharType> THightlightFormatter<CharType>::FindLine(const TDcSourceRef<CharType>& SpanRef)
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

template struct DATACONFIGCORE_API THightlightFormatter<ANSICHAR>;
template struct DATACONFIGCORE_API THightlightFormatter<WIDECHAR>;
