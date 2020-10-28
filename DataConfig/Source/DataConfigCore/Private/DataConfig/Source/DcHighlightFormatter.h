#pragma once
#include "DataConfig/Source/DcSourceTypes.h"

template<class CharType>
struct FHightlightFormatter
{
	using SourceBuf = TDcSourceBuffer<CharType>;
	using SourceRef = TDcSourceRef<CharType>;
	using SourceUtils = TDcCSourceUtils<CharType>;

	static constexpr int _LINE_CONTEXT = 1;

	SourceRef LinesBefore[_LINE_CONTEXT];
	SourceRef LineHighlight;
	SourceRef LinesAfter[_LINE_CONTEXT];

	FString FormatHighlight(const SourceRef& SpanRef, const FDcSourceLocation& Loc);
	SourceRef FindLine(const SourceRef& SpanRef);
};

extern template struct FHightlightFormatter<TCHAR>;


