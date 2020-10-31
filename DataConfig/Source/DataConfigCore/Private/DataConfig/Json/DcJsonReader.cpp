#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "Misc/Parse.h"

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

bool FDcJsonReader::Coercion(EDcDataEntry ToEntry)
{
	if (Token.Type == ETokenType::Number)
	{
		return FDcTypeUtils::IsNumericDataEntry(ToEntry)
			|| ToEntry == EDcDataEntry::String;
	}

	return false;
}

FDcResult FDcJsonReader::ReadNext(EDcDataEntry* OutPtr)
{
	DC_TRY(ConsumeEffectiveToken());

	switch (Token.Type)
	{
	case ETokenType::True: { *OutPtr = EDcDataEntry::Bool; break; }
	case ETokenType::False: { *OutPtr = EDcDataEntry::Bool; break; }
	case ETokenType::CurlyOpen: { *OutPtr = EDcDataEntry::MapRoot; break;}
	case ETokenType::CurlyClose: { *OutPtr = EDcDataEntry::MapEnd; break;}
	case ETokenType::SquareOpen: { *OutPtr = EDcDataEntry::ArrayRoot; break;}
	case ETokenType::SquareClose: { *OutPtr = EDcDataEntry::ArrayEnd; break;}
	case ETokenType::String: { *OutPtr = EDcDataEntry::String; break;}
	case ETokenType::Number: { *OutPtr = EDcDataEntry::Double; break;}
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
	else if (Token.Type == ETokenType::Number)
	{
		ReadOut(OutPtr, Token.Ref.ToString());
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
	Token.Flag.Reset();

	Advance();
	while (true)
	{
		TCharType Char = PeekChar();
		if (Char == _EOF_CHAR)
		{
			return DC_FAIL(DcDJSON, UnclosedStringLiteral) << FormatInputSpan(Token.Ref.Begin, 1);
		}
		else if (Char == TCharType('"'))
		{
			Advance();
			Token.Type = ETokenType::String;
			Token.Ref.Num = Cur - Token.Ref.Begin;
			return DcOk();
		}
		else if (Char == TCharType('\\'))
		{
			Token.Flag.bStringHasEscapeChar = true;

			Advance();
			TCharType EscapeChar = PeekChar();
			if (EscapeChar == TCharType('"'))
				Advance();
		}
		else if (SourceUtils::IsControl(Char))
		{
			return DC_FAIL(DcDJSON, InvalidControlCharInString) << FormatInputSpan(Cur, 1);
		}
		else
		{
			Advance();
		}
	}

	checkNoEntry();
	return DC_FAIL(DcDCommon, Unreachable);
}

FDcResult FDcJsonReader::ParseStringToken(FString &OutStr)
{
	check(Token.Type == ETokenType::String);
	SourceRef literalRef = Token.Ref;
	literalRef.Begin += 1;
	literalRef.Num -= 2;
	if (!Token.Flag.bStringHasEscapeChar)
	{
		OutStr = literalRef.ToString();
		return DcOk();
	}
	else
	{
		// FParse::QuotedString needs string to be quoted
		FString RawStr = Token.Ref.ToString();	
		int32 CharsRead = 0;
		bool bOk = FParse::QuotedString(RawStr.GetCharArray().GetData(), OutStr, &CharsRead);
		if (!bOk)
			return DC_FAIL(DcDJSON, InvalidStringEscaping) << FormatInputSpan(Token.Ref.Begin, CharsRead);
		else
			return DcOk();
	}
}

FDcResult FDcJsonReader::ReadNumberToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	Advance();
	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (Char == TCharType('.'))
		{
			Token.Flag.bNumberHasDecimal = true;
			Advance();
		}
		else if (Char == TCharType('e') || Char == TCharType('E'))
		{
			Token.Flag.bNumberHasExp = true;
			Advance();
		}
		else if (Char == TCharType('-') || Char == TCharType('+') || SourceUtils::IsDigit(Char))
		{
			Advance();
		}
		else
		{
			break;
		}
	}

	Token.Type = ETokenType::Number;
	Token.Ref.Num = Cur - Token.Ref.Begin;
	return DcOk();
}

void FDcJsonReader::ReadWhiteSpace()
{
	Token.Ref.Begin = Cur;

	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
		{
			Loc.Line++;
			Loc.Column = 0;
		}

		if (!SourceUtils::IsWhitespace(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::Whitespace;
}

void FDcJsonReader::ReadLineComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == TCharType('/'));
	check(PeekChar(1) == TCharType('/'));
	AdvanceN(2);

	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::LineComment;
}

FDcResult FDcJsonReader::ReadBlockComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == TCharType('/'));
	check(PeekChar(1) == TCharType('*'));
	AdvanceN(2);

	int Depth = 1;
	while (!IsAtEnd())
	{
		TCharType Char0 = PeekChar(0);
		TCharType Char1 = PeekChar(1);

		if (Char0 == TCharType('/') && Char1 == TCharType('*'))
		{
			Depth += 1;
		}
		else if (Char0 == TCharType('*') && Char1 == TCharType('/'))
		{
			Depth -= 1;
			if (Depth == 0)
			{
				AdvanceN(2);
				break;
			}
		}

		Advance();
	}

	if (Depth != 0)
	{
		return DC_FAIL(DcDJSON, UnclosedBlockComment) << FormatInputSpan(Token.Ref.Begin, 2);
	}
	else
	{
		Token.Ref.Num = Cur - Token.Ref.Begin;
		Token.Type = ETokenType::BlockComment;
		return DcOk();
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
			DC_TRY(ConsumeEffectiveToken());

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
			DC_TRY(ConsumeEffectiveToken());
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
		DC_TRY(ConsumeEffectiveToken());

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

template<typename TInt>
FDcResult FDcJsonReader::ParseInteger(TInt& OutInt)
{
	return DcOk();
}

FDcResult FDcJsonReader::ReadInt32(int32* OutPtr)
{
	int32 Value;
	ParseInteger<int32>(Value);
	return DcOk();
}

FDcResult FDcJsonReader::ReadUInt32(uint32* OutPtr)
{
	return DcOk();
}

FDcResult FDcJsonReader::ReadDouble(double* OutPtr)
{
	return DcOk();
}

FDcResult FDcJsonReader::ConsumeRawToken()
{
	check(State == EState::InitializedWithStr);
	if (CachedNext.IsValid())
	{
		Token = CachedNext;
		CachedNext.Reset();
		return DcOk();
	}

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
	if (SourceUtils::IsWhitespace(Char))
	{
		ReadWhiteSpace();
		return DcOk();
	}
	else if (Char == TCharType('/'))
	{
		TCharType NextChar = PeekChar(1);
		if (NextChar == TCharType('/'))
		{
			ReadLineComment();
			return DcOk();
		}
		else if (NextChar == TCharType('*'))
		{
			return ReadBlockComment();
		}
		else
		{
			return DC_FAIL(DcDJSON, UnexpectedChar) << Char << FormatInputSpan(Cur, 1);
		}
	}
	else if (Char == TCharType('{'))
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
	else if (Char == TCharType('-')
		|| SourceUtils::IsDigit(Char))
	{
		return ReadNumberToken();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedChar)
			<< FString::Chr(Char) << FormatInputSpan(Cur, 1);
	}
}

FDcResult FDcJsonReader::ConsumeEffectiveToken()
{
	while (true)
	{
		FDcResult Ret = ConsumeRawToken();
		if (!Ret.Ok())
			return Ret;
		else if (Token.Type < ETokenType::LineComment)
			return DcOk();
		else
			continue;
	}

	checkNoEntry();
	return DC_FAIL(DcDCommon, Unreachable);
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

FDcJsonReader::TCharType FDcJsonReader::PeekChar(int N)
{
	if (IsAtEnd(N))
		return _EOF_CHAR;
	else
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


FDcDiagnosticHighlight FDcJsonReader::FormatInputSpan(SourceRef SpanRef)
{
	FDcDiagnosticHighlight OutHighlight;
	OutHighlight.Loc = Loc;
	OutHighlight.FilePath = DiagFilePath;
	FHightlightFormatter<TCharType> Highlighter;
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, Loc);

	return OutHighlight;
}

FDcDiagnosticHighlight FDcJsonReader::FormatInputSpan(int Begin, int Num)
{
	return FormatInputSpan(SourceRef{ &Buf, Begin, Num });
}

