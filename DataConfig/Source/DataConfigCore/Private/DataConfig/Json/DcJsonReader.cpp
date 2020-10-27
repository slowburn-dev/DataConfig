#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "Misc/StringBuilder.h"


FDcJsonReader::FDcJsonReader(const FString* InStrPtr)
	: FDcJsonReader()
{
	SetNewString(InStrPtr);
}

FDcJsonReader::FDcJsonReader()
{
	States.Add(EParseState::Nil);
}

void FDcJsonReader::SetNewString(const FString* InStrPtr)
{
	check(State == EState::Unitialized || State == EState::FinishedStr);
	Buf = SourceBuf(InStrPtr->GetCharArray().GetData());
	Token.Type = ETokenType::EOF_;
	Token.Ref.Reset();
	Token.Ref.Buffer = &Buf;

	State = EState::InitializedWithStr;
	Cur = 0;
	Loc.Line = 0;
	Loc.Column = 0;
}

FDcResult FDcJsonReader::PeekRead(EDcDataEntry* OutPtr)
{
	DC_TRY(ConsumeToken());

	switch (Token.Type)
	{
	case ETokenType::True: { *OutPtr = EDcDataEntry::Bool; break; }
	case ETokenType::False: { *OutPtr = EDcDataEntry::Bool; break; }
	case ETokenType::CurlyOpen: { *OutPtr = EDcDataEntry::MapRoot; break;}
	case ETokenType::CurlyClose: { *OutPtr = EDcDataEntry::MapEnd; break;}
	case ETokenType::SquareOpen: { *OutPtr = EDcDataEntry::ArrayRoot; break;}
	case ETokenType::SquareClose: { *OutPtr = EDcDataEntry::ArrayEnd; break;}
	case ETokenType::String: { *OutPtr = EDcDataEntry::String; break;}
	case ETokenType::EOF_: { *OutPtr = EDcDataEntry::Ended; break;}
	default: 
		return DC_FAIL(DcDCommon, NotImplemented);
	}

	return DcOk();
}

FDcResult FDcJsonReader::ReadBool(bool* OutPtr)
{
	if (Token.Type == ETokenType::True)
	{
		ReadOut(OutPtr, true);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else if (Token.Type == ETokenType::False)
	{
		ReadOut(OutPtr, false);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadName(FName* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));
		ReadOut(OutPtr, *ParsedStr);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadString(FString* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));
		ReadOut(OutPtr, ParsedStr);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadStringToken()
{
	Token.Ref.Begin = Cur;
	Advance();
	while (true)
	{
		if (IsAtEnd())
			return DcFail();

		TCharType Char = ReadChar();
		check(Char != TCharType('\0'));	// should be handled in IsAtEnd();
		if (Char == TCharType('"'))
		{
			Token.Type = ETokenType::String;
			Token.Ref.Num = Cur - Token.Ref.Begin;
			return DcOk();
		}
		else
		{
			//	TODO handle scaping and things
			//	pass
		}
	}

	checkNoEntry();
	return DC_FAIL(DcDCommon, Unreachable);
}

FDcResult FDcJsonReader::ParseStringToken(FString &OutStr)
{
	//	for now simply strip the quotes
	check(Token.Type == ETokenType::String);
	SourceRef literalRef = Token.Ref;
	literalRef.Begin += 1;
	literalRef.Num -= 2;
	OutStr = literalRef.ToString();

	return DcOk();
}

void FDcJsonReader::ReadWhiteSpace()
{
	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
		{
			Cur++;

			Loc.Line++;
			Loc.Column = 0;
		}

		if (SourceUtils::IsWhitespace(Char))
		{
			Advance();
		}
		else
		{
			break;
		}
	}
}

FDcResult FDcJsonReader::EndTopRead()
{
	EParseState TopState = GetTopState();
	if (TopState == EParseState::Object)
	{
		if (!bTopObjectAtValue)
		{
			//	at key position
			DC_TRY(ConsumeToken());

			if (Token.Type != ETokenType::Colon)
			{
				return DC_FAIL(DcDJSON, UnexpectedToken);
			}

			bTopObjectAtValue = true;
			return DcOk();
		}
		else
		{
			//	at value position
			FToken Prev = Token;
			DC_TRY(ConsumeToken());
			bTopObjectAtValue = false;

			//	allowing optional trailing comma
			if (Token.Type == ETokenType::Comma)
			{
				return DcOk();
			}
			else if (Token.Type == ETokenType::CurlyClose)
			{
				PutbackToken(Prev);
				return DcOk();
			}
			else
			{
				return DC_FAIL(DcDJSON, UnexpectedToken);
			}
		}
	}
	else if (TopState == EParseState::Array)
	{
		FToken Prev = Token;
		DC_TRY(ConsumeToken());

		if (Token.Type == ETokenType::Comma)
		{
			return DcOk();
		}
		else if (Token.Type == ETokenType::SquareClose)
		{
			PutbackToken(Prev);
			return DcOk();
		}
		else
		{
			return DC_FAIL(DcDJSON, UnexpectedToken);
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

FDcResult FDcJsonReader::ReadMapRoot()
{
	if (Token.Type == ETokenType::CurlyOpen)
	{
		PushTopState(EParseState::Object);
		bTopObjectAtValue = false;
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadMapEnd()
{
	if (Token.Type == ETokenType::CurlyClose)
	{
		PopTopState(EParseState::Object);
		//	!!! HACK
		//	we know it's always at value position due to JSON spec, so just set it
		bTopObjectAtValue = true;
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadArrayRoot()
{
	if (Token.Type == ETokenType::SquareOpen)
	{
		PushTopState(EParseState::Array);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ReadArrayEnd()
{
	if (Token.Type == ETokenType::SquareClose)
	{
		PopTopState(EParseState::Array);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
}

FDcResult FDcJsonReader::ConsumeToken()
{
	if (CachedNext.IsValid())
	{
		Token = CachedNext;
		CachedNext.Reset();
		return DcOk();
	}

	check(State == EState::InitializedWithStr);
	ReadWhiteSpace();

	if (IsAtEnd())
	{
		Token.Type = ETokenType::EOF_;
		Token.Ref.Reset();
		return DcOk();
	}

	auto _ConsumeSingleCharToken = [this](ETokenType TokenType) {
		Token.Type = TokenType;
		Token.Ref.Begin = Cur;
		Token.Ref.Num = 1;
		Advance();
		return DcOk();
	};

	TCharType Char = PeekChar();
	if (Char == TCharType('{'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyOpen);
	}
	else if (Char == TCharType('}'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyClose);
	}
	else if (Char == TCharType('['))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareOpen);
	}
	else if (Char == TCharType(']'))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareClose);
	}
	else if (Char == TCharType(':'))
	{
		return _ConsumeSingleCharToken(ETokenType::Colon);
	}
	else if (Char == TCharType(','))
	{
		return _ConsumeSingleCharToken(ETokenType::Comma);
	}
	else if (Char == TCharType('t'))
	{
		DC_TRY(ReadWordExpect(_TRUE_LITERAL));
		Token.Type = ETokenType::True;
		return DcOk();
	}
	else if (Char == TCharType('f'))
	{
		DC_TRY(ReadWordExpect(_FALSE_LITERAL));
		Token.Type = ETokenType::False;
		return DcOk();
	}
	else if (Char == TCharType('n'))
	{
		DC_TRY(ReadWordExpect(_NULL_LITERAL));
		Token.Type = ETokenType::Null;
		return DcOk();
	}
	else if (Char == TCharType('"'))
	{
		return ReadStringToken();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedChar)
			<< FString::Chr(Char) << FormatInputSpan(Cur, 1);
	}
}

void FDcJsonReader::PutbackToken(const FToken& Putback)
{
	check(Token.IsValid() && Putback.IsValid());
	check(!CachedNext.IsValid());
	CachedNext = Token;
	Token = Putback;
}

bool FDcJsonReader::IsAtEnd(int N)
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur + N >= Buf.Num;
}

void FDcJsonReader::Advance()
{
	check(!IsAtEnd());
	++Cur;
	Loc.Column++;
}

void FDcJsonReader::AdvanceN(int N)
{
	check(N != 0);
	check(!IsAtEnd(N));
	Cur += N;
	Loc.Column += N;
}

FDcJsonReader::TCharType FDcJsonReader::ReadChar()
{
	check(!IsAtEnd());
	TCharType Ret = PeekChar();
	Advance();
	return Ret;
}

FDcJsonReader::TCharType FDcJsonReader::PeekChar(int N)
{
	check(!IsAtEnd(N));
	return Buf.Buffer[Cur + N];
}

FDcResult FDcJsonReader::ReadWordExpect(const TCharType* Word)
{
	int32 RefBegin = Cur;
	int32 WordLen = CString::Strlen(Word);
	SourceRef WordRef = Token.Ref;
	WordRef.Begin = Cur;
	WordRef.Num = WordLen;

	if (IsAtEnd(WordLen))
	{
		return DcFail();
	}

	for (int Ix = 0; Ix < WordLen; Ix++)
	{
		if (TCharType(Word[Ix]) != PeekChar(Ix))
		{
			return DC_FAIL(DcDJSON, ExpectWordButNotFound)
				<< Word << WordRef.ToString()
				<< FormatInputSpan(WordRef);
		}
	}

	Token.Ref = WordRef;
	AdvanceN(WordLen);
	return DcOk();
}

struct FHightlightFormatter
{
	using TReader = FDcJsonReader;
	using SourceBuf = TReader::SourceBuf;
	using SourceRef = TReader::SourceRef;

	static constexpr int _LINE_CONTEXT = 1;

	SourceRef LinesBefore[_LINE_CONTEXT];
	SourceRef LineHighlight;
	SourceRef LinesAfter[_LINE_CONTEXT];

	FString FormatHighlight(const SourceRef& SpanRef, const FDcSourceLocation& Loc);
	SourceRef FindLine(const SourceRef& SpanRef);
};


FString FHightlightFormatter::FormatHighlight(const SourceRef& SpanRef, const FDcSourceLocation& Loc)
{
	LineHighlight = FindLine(SpanRef);
	check(LineHighlight.IsValid());

	{
		SourceRef LineBefore = LineHighlight;
		for (int Ix = 0; Ix < _LINE_CONTEXT; Ix++)
		{
			LineBefore.Begin = LineBefore.Begin - 2;	// -1 is \n
			LineBefore.Num = 0;
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
			LineAfter.Num = 0;
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

FHightlightFormatter::SourceRef FHightlightFormatter::FindLine(const SourceRef& SpanRef)
{
	check(SpanRef.IsValid());

	const SourceBuf* Buf = SpanRef.Buffer;
	int32 CurHead = SpanRef.Begin;
	while (CurHead >= 0)
	{
		if (TReader::SourceUtils::IsLineBreak(Buf->Get(CurHead)))
		{
			++CurHead;
			break;
		}
		--CurHead;
	}

	int32 CurTail = SpanRef.Begin;
	while (CurTail < Buf->Num)
	{
		if (TReader::SourceUtils::IsLineBreak(Buf->Get(CurTail++)))
			break;
	}

	return SourceRef{ Buf, CurHead, CurTail - CurHead };
}

FDcDiagnosticHighlight FDcJsonReader::FormatInputSpan(SourceRef SpanRef)
{
	FDcDiagnosticHighlight OutHighlight;
	OutHighlight.Loc = Loc;
	OutHighlight.FilePath = DiagFilePath;
	FHightlightFormatter Highlighter;
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, Loc);

	return OutHighlight;
}

FDcDiagnosticHighlight FDcJsonReader::FormatInputSpan(int Begin, int Num)
{
	return FormatInputSpan(SourceRef{ &Buf, Begin, Num });
}

