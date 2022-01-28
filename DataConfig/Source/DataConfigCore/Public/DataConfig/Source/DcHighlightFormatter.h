#pragma once

#include "DataConfig/Source/DcSourceTypes.h"
#include "DataConfig/Source/DcSourceUtils.h"

template<class CharType>
struct THightlightFormatter
{
	using SourceBuf = TDcSourceView<CharType>;
	using SourceRef = TDcSourceRef<CharType>;
	using SourceUtils = TDcCSourceUtils<CharType>;

	static constexpr int _LINE_CONTEXT = 2;
	static constexpr int _LINE_MAX_LENGH = 140;

	SourceRef LinesBefore[_LINE_CONTEXT];
	SourceRef LineHighlight;
	SourceRef LinesAfter[_LINE_CONTEXT];

	FString FormatHighlight(SourceRef SpanRef, int Line = INDEX_NONE, FString* ReportFirstLine = nullptr);

	static SourceRef FindLine(const SourceRef& SpanRef);
	static int FindLineNumber(const SourceRef& SpanRef);
	static FString FormatDiagnosticLine(const FString& InLine);
};


