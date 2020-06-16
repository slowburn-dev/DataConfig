#include "Json/DcJsonReader.h"

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

	return EDataEntry::Ended;
}

FResult FJsonReader::ReadBool(bool* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();

	if (PeekChar() == TCharType('t'))
	{
		TRY(ReadWordExpect(TEXT("true"), EErrorCode::ReadBoolFail));

		if (OutPtr)
		{
			*OutPtr = true;
		}

		EndTopRead();
		return Ok();
	}
	else if (PeekChar() == 'f')
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
		return Fail(EErrorCode::ReadBoolFail);
	}
}

FResult FJsonReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();

	if (PeekChar() == TCharType('"'))
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
		return Fail(EErrorCode::ReadNameFail);
	}
}

FResult FJsonReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	ReadWhiteSpace();

	if (PeekChar() == TCharType('"'))
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
		return Fail(EErrorCode::ReadStringFail);
	}
}

DataConfig::FResult FJsonReader::ReadString(FString& OutStr)
{
	ReadWhiteSpace();

	TRY(ReadCharExpect(TCharType('"'), EErrorCode::ReadStringFail));

	while (true)
	{
		if (IsAtEnd())
			return Fail(EErrorCode::ReadStringFail);

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
	return Fail(EErrorCode::UnknownError);
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
				return Fail(EErrorCode::ReadMapFail);
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
			return Fail(EErrorCode::ReadArrayFail);
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return Ok();
	}
	else
	{
		checkNoEntry();
		return Fail(EErrorCode::UnknownError);
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
		return Fail(ErrCode);

	OutChar = PeekChar();
	return Ok();
}

FResult FJsonReader::ReadWordExpect(const TCharType* Word, EErrorCode ErrCode)
{
	while (true)
	{
		if (IsAtEnd()) return Fail(ErrCode);
		if (*Word == TCharType('\0')) return Ok();
		if (*Word != ReadChar()) return Fail(ErrCode);

		++Word;
	}

	checkNoEntry();
	return Fail(EErrorCode::UnknownError);
}

FResult FJsonReader::ReadCharExpect(TCharType Expect, EErrorCode ErrCode)
{
	if (IsAtEnd()) 
		return Fail(ErrCode);
	return ReadChar() == Expect ? Ok() : Fail(ErrCode);
}

} // namespace DataConfig

