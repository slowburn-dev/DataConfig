#include "Json/DcJsonReader.h"

namespace DataConfig
{

using TCharType = FJsonReader::TCharType;

FJsonReader::FJsonReader(const FString* InStrPtr)
	: FJsonReader()
{
	SetNewString(InStrPtr);
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
	if (IsAtEnd())
		return EDataEntry::Ended;

	//	consume whitespace

	TCharType Char = ReadChar();
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
	ReadCharExpect(TCharType('o'), EErrorCode::ReadBoolFail);
	ReadCharExpect(TCharType('o'), EErrorCode::ReadBoolFail);
	ReadCharExpect(TCharType('l'), EErrorCode::ReadBoolFail);

	return Ok();
}

FResult FJsonReader::ReadName(FName* OutPtr, FContextStorage* CtxPtr)
{
	return Ok();
}

FResult FJsonReader::ReadString(FString* OutPtr, FContextStorage* CtxPtr)
{
	return Ok();
}

FResult FJsonReader::ReadMapRoot(FContextStorage* CtxPtr)
{
	return Ok();
}

FResult FJsonReader::ReadMapEnd(FContextStorage* CtxPtr)
{
	return Ok();
}

bool FJsonReader::IsAtEnd()
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur < StrPtr->Len();
}

TCharType FJsonReader::ReadChar()
{
	check(!IsAtEnd());
	return (*StrPtr)[Cur++];
}

void FJsonReader::PutBack()
{
	check(Cur > 0);
	--Cur;
}

DataConfig::FResult FJsonReader::ReadCharExpect(TCharType Expect, EErrorCode ErrCode)
{
	if (IsAtEnd()) 
		return Fail(ErrCode);
	return ReadChar() == Expect ? Ok() : Fail(ErrCode);
}

} // namespace DataConfig

