#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"

namespace DataConfig
{

using TCharType = FJsonReader::TCharType;

FJsonReader::FJsonReader(const FString* InStrPtr)
	: FJsonReader()
{
	SetNewString(InStrPtr);
}

FJsonReader::FJsonReader()
{
	States.Add(EParseState::Nil);
}

void FJsonReader::SetNewString(const FString* InStrPtr)
{
	check(State == EState::Unitialized || State == EState::FinishedStr);
	StrPtr = InStrPtr;
	State = EState::InitializedWithStr;
	Cur = 0;
}

EDataEntry FJsonReader::Peek()
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

FResult FJsonReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();
	//	TODO end check

	TCharType Next = PeekChar();
	if (Next == TCharType('t'))
	{
		TRY(ReadWordExpect(TEXT("true"), EErrorCode::ReadBoolFail));

		if (OutPtr)
		{
			*OutPtr = true;
		}

		EndTopRead();
		return Ok();
	}
	else if (Next == 'f')
	{
		TRY(ReadWordExpect(TEXT("false"), EErrorCode::ReadBoolFail));

		if (OutPtr)
		{
			*OutPtr = false;
		}

		EndTopRead();
		return Ok();
	}
	else
	{
		return Fail(DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

FResult FJsonReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();
	//	TODO end check

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = FName(*Str);
		}

		EndTopRead();
		return Ok();
	}
	else
	{
		return Fail(DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

FResult FJsonReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();

	TCharType Next = PeekChar();
	if (Next == TCharType('"'))
	{
		FString Str;
		TRY(ReadString(Str));

		if (OutPtr)
		{
			*OutPtr = *Str;
		}

		EndTopRead();
		return Ok();
	}
	else
	{
		return Fail(DIAG(DJSON, UnexpectedChar1)) << Next;
	}
}

DataConfig::FResult FJsonReader::ReadString(FString& OutStr)
{
	ReadWhiteSpace();

	TRY(ReadCharExpect(TCharType('"'), EErrorCode::ReadStringFail));

	while (true)
	{
		if (IsAtEnd())
			return Fail(DIAG(DJSON, UnexpectedEnd));

		TCharType Char = ReadChar();
		check(Char != TCharType('\0'));	// should be handled in IsAtEnd();
		if (Char == TCharType('"'))
		{
			return Ok();
		}
		else
		{
			OutStr += Char;
		}
	}

	checkNoEntry();
	return Fail(DIAG(DCommon, Unreachable));
}

void FJsonReader::ReadWhiteSpace()
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

DataConfig::FResult FJsonReader::EndTopRead()
{
	EParseState TopState = GetTopState();
	if (TopState == EParseState::Object)
	{
		ReadWhiteSpace();

		if (!bTopObjectAtValue)
		{
			bTopObjectAtValue = true;
			TRY(ReadCharExpect(TCharType(':'), EErrorCode::ReadMapFail));
			return Ok();
		}
		else
		{
			bTopObjectAtValue = false;
			TCharType Char;
			TRY(PeekChar(Char, EErrorCode::ReadMapFail));

			if (Char == TCharType(','))
			{
				Advance();
				return Ok();
			}
			else if (Char == TCharType('}'))
			{
				//	pass
				return Ok();
			}
			else
			{
				return Fail(DIAG(DJSON, UnexpectedChar1)) << Char;
			}
		}
	}
	else if (TopState == EParseState::Array)
	{
		TCharType Char;
		TRY(PeekChar(Char, EErrorCode::ReadArrayFail));

		if (Char == TCharType(','))
		{
			Advance();
			return Ok();
		}
		else if (Char == TCharType(']'))
		{
			//	pass
			return Ok();
		}
		else
		{
			return Fail(DIAG(DJSON, UnexpectedChar1)) << Char;
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return Ok();
	}
	else
	{
		checkNoEntry();
		return Fail(DIAG(DCommon, Unreachable));
	}
}

FResult FJsonReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	TRY(ReadCharExpect(TCharType('{'), EErrorCode::ReadMapFail));
	PushTopState(EParseState::Object);
	bTopObjectAtValue = false;

	return Ok();
}

FResult FJsonReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	TRY(ReadCharExpect(TCharType('}'), EErrorCode::ReadMapEndFail));
	PopTopState(EParseState::Object);

	//	!!! HACK
	//	we know it's always at value position due to JSON spec, so just set it
	bTopObjectAtValue = true;

	TRY(EndTopRead());
	return Ok();
}

bool FJsonReader::IsAtEnd()
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur >= StrPtr->Len();
}

void FJsonReader::Advance()
{
	check(!IsAtEnd());
	++Cur;
}

TCharType FJsonReader::ReadChar()
{
	check(!IsAtEnd());
	return (*StrPtr)[Cur++];
}

TCharType FJsonReader::PeekChar()
{
	check(!IsAtEnd());
	return (*StrPtr)[Cur];
}

FResult FJsonReader::PeekChar(TCharType& OutChar, EErrorCode ErrCode)
{
	if (IsAtEnd())
		return Fail(DIAG(DJSON, UnexpectedEnd));

	OutChar = PeekChar();
	return Ok();
}

FResult FJsonReader::ReadWordExpect(const TCharType* Word, EErrorCode ErrCode)
{
	while (true)
	{
		if (IsAtEnd())
			return Fail(DIAG(DJSON, AlreadyEndedButExpect)) << Word;
		if (*Word == TCharType('\0'))
			return Ok();	// !!! note that this is end of `Word`
		if (*Word != ReadChar())
			return Fail(DIAG(DJSON, ExpectWordButNotFound)) << Word;

		++Word;
	}

	checkNoEntry();
	return Fail(DIAG(DCommon, Unreachable));
}

FResult FJsonReader::ReadCharExpect(TCharType Expect, EErrorCode ErrCode)
{
	if (IsAtEnd())
		return Fail(ErrCode);
	return ReadChar() == Expect 
		? Ok()
		: Fail(DIAG(DJSON, ExpectCharButNotFound)) << Expect;
}

} // namespace DataConfig

