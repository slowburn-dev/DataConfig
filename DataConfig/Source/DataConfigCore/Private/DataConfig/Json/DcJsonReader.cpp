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
	State = EState::InitializedWithStr;
	Cur = 0;
	LineStart = 0;
	Loc.Line = 0;
	Loc.Column = 0;
}

EDcDataEntry FDcJsonReader::Peek()
{
	switch (Token.Type)
	{
	case ETokenType::True: return EDcDataEntry::Bool;
	case ETokenType::False: return EDcDataEntry::Bool;
	case ETokenType::CurlyOpen: return EDcDataEntry::MapRoot;
	case ETokenType::CurlyClose: return EDcDataEntry::MapEnd;
	case ETokenType::String: return EDcDataEntry::String;
	case ETokenType::EOF_: return EDcDataEntry::Ended;
		default: return EDcDataEntry::Ended;
	}
}

FDcResult FDcJsonReader::ReadBool(bool* OutPtr)
{
	if (Token.Type == ETokenType::True)
	{
		*OutPtr = true;
	}
	else if (Token.Type == ETokenType::False)
	{
		*OutPtr = false;
	}

	return DcOk();
}

FDcResult FDcJsonReader::ReadName(FName* OutPtr)
{
	ReadWhiteSpace();
	//	TODO end check

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		//DC_TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = FName(*Str);
		}

		EndTopRead();
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDJSON, UnexpectedChar1)) << Next;
	}
}

FDcResult FDcJsonReader::ReadString(FString* OutPtr)
{
	ReadWhiteSpace();

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		//DC_TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = *Str;
		}

		EndTopRead();
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DcDJSON, UnexpectedChar1)) << Next;
	}
}

FDcResult FDcJsonReader::ReadString(FString& OutStr)
{
	ReadWhiteSpace();

	DC_TRY(ReadCharExpect(TCharType('"')));

	while (true)
	{
		if (IsAtEnd())
			return DcFail(DC_DIAG(DcDJSON, UnexpectedEnd));

		TCharType Char = ReadChar();
		check(Char != TCharType('\0'));	// should be handled in IsAtEnd();
		if (Char == TCharType('"'))
		{
			return DcOk();
		}
		else
		{
			OutStr += Char;
		}
	}

	checkNoEntry();
	return DcFail(DC_DIAG(DcDCommon, Unreachable));
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
			Advance();
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
		ReadWhiteSpace();

		if (!bTopObjectAtValue)
		{
			bTopObjectAtValue = true;
			DC_TRY(ReadCharExpect(TCharType(':')));
			return DcOk();
		}
		else
		{
			bTopObjectAtValue = false;
			TCharType Char;
			DC_TRY(TryPeekChar(Char));

			if (Char == TCharType(','))
			{
				Advance();
				return DcOk();
			}
			else if (Char == TCharType('}'))
			{
				//	pass
				return DcOk();
			}
			else
			{
				return DcFail(DC_DIAG(DcDJSON, UnexpectedChar1)) << Char;
			}
		}
	}
	else if (TopState == EParseState::Array)
	{
		TCharType Char;
		DC_TRY(TryPeekChar(Char));

		if (Char == TCharType(','))
		{
			Advance();
			return DcOk();
		}
		else if (Char == TCharType(']'))
		{
			//	pass
			return DcOk();
		}
		else
		{
			return DcFail(DC_DIAG(DcDJSON, UnexpectedChar1)) << Char;
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DcDCommon, Unreachable));
	}
}

FDcInputSpan FDcJsonReader::FormatInputSpan()
{
	FDcInputSpan OutSpan;
	return OutSpan;
}

FDcResult FDcJsonReader::ReadMapRoot()
{
	DC_TRY(ReadCharExpect(TCharType('{')));
	PushTopState(EParseState::Object);
	bTopObjectAtValue = false;

	return DcOk();
}

FDcResult FDcJsonReader::ReadMapEnd()
{
	DC_TRY(ReadCharExpect(TCharType('}')));
	PopTopState(EParseState::Object);

	//	!!! HACK
	//	we know it's always at value position due to JSON spec, so just set it
	bTopObjectAtValue = true;

	DC_TRY(EndTopRead());
	return DcOk();
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

	TCharType Char = PeekChar();
	if (Char == TCharType('{'))
	{
		Token.Type = ETokenType::CurlyOpen;
		Token.Ref.Begin = Cur;
		Token.Ref.Num = 1;
		Advance();
		return DcOk();
	}
	else if (Char == TCharType('}'))
	{
		Token.Type = ETokenType::CurlyClose;
		Token.Ref.Begin = Cur;
		Token.Ref.Num = 1;
		Advance();
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
		return DcOk();
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

FDcResult FDcJsonReader::TryPeekChar(TCharType& OutChar)
{
	if (IsAtEnd())
		return DcFail(DC_DIAG(DcDJSON, UnexpectedEnd));

	OutChar = PeekChar();
	return DcOk();
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

FDcResult FDcJsonReader::ReadCharExpect(TCharType Expect)
{
	if (IsAtEnd())
		return DcFail();
	return ReadChar() == Expect 
		? DcOk()
		: DcFail(DC_DIAG(DcDJSON, ExpectCharButNotFound)) << Expect;
}

