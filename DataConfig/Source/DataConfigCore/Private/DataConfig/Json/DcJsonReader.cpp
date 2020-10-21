#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"

using TCharType = FDcJsonReader::TCharType;

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
	StrPtr = InStrPtr;
	State = EState::InitializedWithStr;
	Cur = 0;
}

EDataEntry FDcJsonReader::Peek()
{
	ReadWhiteSpace();
	if (IsAtEnd())
		return EDataEntry::Ended;

	TCharType Char = PeekChar();
	if (Char == TCharType('t'))	// true
		return EDataEntry::Bool;
	else if (Char == TCharType('f')) // false
		return EDataEntry::Bool;
	else if (Char == TCharType('{'))
		return EDataEntry::MapRoot;
	else if (Char == TCharType('}'))
		return EDataEntry::MapEnd;
	else if (Char == TCharType('"'))
		return EDataEntry::String;
	else {
#if DO_CHECK
		PLATFORM_BREAK();
#endif
		return EDataEntry::Ended;
	}

	return EDataEntry::Ended;
}

FDcResult FDcJsonReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();
	//	TODO end check

	TCharType Next = PeekChar();
	if (Next == TCharType('t'))
	{
		DC_TRY(ReadWordExpect(TEXT("true")));

		if (OutPtr)
		{
			*OutPtr = true;
		}

		EndTopRead();
		return DcOk();
	}
	else if (Next == 'f')
	{
		DC_TRY(ReadWordExpect(TEXT("false")));

		if (OutPtr)
		{
			*OutPtr = false;
		}

		EndTopRead();
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

FDcResult FDcJsonReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();
	//	TODO end check

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		DC_TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = FName(*Str);
		}

		EndTopRead();
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

FDcResult FDcJsonReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		DC_TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = *Str;
		}

		EndTopRead();
		return DcOk();
	}
	else
	{
		return DcFail(DC_DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

FDcResult FDcJsonReader::ReadString(FString& OutStr)
{
	ReadWhiteSpace();

	DC_TRY(ReadCharExpect(TCharType('"')));

	while (true)
	{
		if (IsAtEnd())
			return DcFail(DC_DIAG(DJSON, UnexpectedEnd));

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
	return DcFail(DC_DIAG(DCommon, Unreachable));
}

void FDcJsonReader::ReadWhiteSpace()
{
	//	TODO actually can add a dirty flag to save duplicated read white space
	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (IsLineBreak(Char))
		{
			//	pass for now, add line count
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
				return DcFail(DC_DIAG(DJSON, UnexpectedChar1)) << Char;
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
			return DcFail(DC_DIAG(DJSON, UnexpectedChar1)) << Char;
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DcFail(DC_DIAG(DCommon, Unreachable));
	}
}

FDcResult FDcJsonReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	DC_TRY(ReadCharExpect(TCharType('{')));
	PushTopState(EParseState::Object);
	bTopObjectAtValue = false;

	return DcOk();
}

FDcResult FDcJsonReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	DC_TRY(ReadCharExpect(TCharType('}')));
	PopTopState(EParseState::Object);

	//	!!! HACK
	//	we know it's always at value position due to JSON spec, so just set it
	bTopObjectAtValue = true;

	DC_TRY(EndTopRead());
	return DcOk();
}

bool FDcJsonReader::IsAtEnd()
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur >= StrPtr->Len();
}

void FDcJsonReader::Advance()
{
	check(!IsAtEnd());
	++Cur;
}

TCharType FDcJsonReader::ReadChar()
{
	check(!IsAtEnd());
	return (*StrPtr)[Cur++];
}

TCharType FDcJsonReader::PeekChar()
{
	check(!IsAtEnd());
	return (*StrPtr)[Cur];
}

FDcResult FDcJsonReader::TryPeekChar(TCharType& OutChar)
{
	if (IsAtEnd())
		return DcFail(DC_DIAG(DJSON, UnexpectedEnd));

	OutChar = PeekChar();
	return DcOk();
}

FDcResult FDcJsonReader::ReadWordExpect(const TCharType* Word)
{
	while (true)
	{
		if (IsAtEnd())
			return DcFail(DC_DIAG(DJSON, AlreadyEndedButExpect)) << Word;
		if (*Word == TCharType('\0'))
			return DcOk();	// !!! note that this is end of `Word`
		if (*Word != ReadChar())
			return DcFail(DC_DIAG(DJSON, ExpectWordButNotFound)) << Word;

		++Word;
	}

	checkNoEntry();
	return DcFail(DC_DIAG(DCommon, Unreachable));
}

FDcResult FDcJsonReader::ReadCharExpect(TCharType Expect)
{
	if (IsAtEnd())
		return DcFail();
	return ReadChar() == Expect 
		? DcOk()
		: DcFail(DC_DIAG(DJSON, ExpectCharButNotFound)) << Expect;
}

