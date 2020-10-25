#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"


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
	Token.Ref.SourceBuffer = &Buf;

	State = EState::InitializedWithStr;
	Cur = 0;
	LineStart = 0;
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
	//	TODO actually can add a dirty flag to save duplicated read white space
	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (IsLineBreak(Char))
		{
			Cur++;
			LineStart = Cur;

			Loc.Line++;
			Loc.Column = 0;
		}

		if (IsWhitespace(Char))
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
			DC_TRY(ConsumeToken());
			bTopObjectAtValue = false;

			if (Token.Type == ETokenType::Comma)
			{
				return DcOk();
			}
			else if (Token.Type == ETokenType::CurlyClose)
			{
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
		//	TODO these block isn't evaluated yet
		DC_TRY(ConsumeToken());

		if (Token.Type == ETokenType::Comma)
		{
			return DcOk();
		}
		else if (Token.Type == ETokenType::SquareClose)
		{
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

FDcInputSpan FDcJsonReader::FormatInputSpan()
{
	FDcInputSpan OutSpan;
	return OutSpan;
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

FDcResult FDcJsonReader::ConsumeToken()
{
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
	};

	TCharType Char = PeekChar();
	if (Char == TCharType('{'))
	{
		_ConsumeSingleCharToken(ETokenType::CurlyOpen);
		return DcOk();
	}
	else if (Char == TCharType('}'))
	{
		_ConsumeSingleCharToken(ETokenType::CurlyClose);
		return DcOk();
	}
	else if (Char == TCharType(':'))
	{
		_ConsumeSingleCharToken(ETokenType::Colon);
		return DcOk();
	}
	else if (Char == TCharType(','))
	{
		_ConsumeSingleCharToken(ETokenType::Comma);
		return DcOk();
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
		//	TODO fail with unexpected
		return DC_FAIL(DcDJSON, UnexpectedToken);
	}
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
			return DcFail();
		}
	}

	Token.Ref = WordRef;
	AdvanceN(WordLen);
	return DcOk();
}

