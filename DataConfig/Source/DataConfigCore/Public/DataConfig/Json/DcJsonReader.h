#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FJsonReader : public FReader, private FNoncopyable
{
	using TCharType = TCHAR;

	FJsonReader();
	FJsonReader(const FString* InStrPtr);

	void SetNewString(const FString* InStrPtr);

	enum class EState
	{
		Unitialized,
		InitializedWithStr,
		FinishedStr,
		Invalid,
	};
	EState State = EState::Unitialized;

	const FString* StrPtr = nullptr;
	int32 Cur = 0;

	EDataEntry Peek() override;

	FResult ReadBool(bool* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadName(FName* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadString(FString* OutPtr, FContextStorage* CtxPtr) override;
	FResult ReadMapRoot(FContextStorage* CtxPtr) override;
	FResult ReadMapEnd(FContextStorage* CtxPtr) override;

	bool IsAtEnd();
	void Advance();
	TCharType ReadChar();
	TCharType PeekChar();

	FResult PeekChar(TCharType& OutChar, EErrorCode ErrCode);
	FResult ReadWordExpect(const TCharType* Word, EErrorCode ErrCode);
	FResult ReadCharExpect(TCharType Expect, EErrorCode ErrCode);
	FResult ReadString(FString& OutStr);

	void ReadWhiteSpace();

	enum class EParseState
	{
		Nil,
		Object,
		Array,
	};

	TArray<EParseState, TInlineAllocator<8>> States;
	FORCEINLINE EParseState GetTopState();
	FORCEINLINE void PushTopState(EParseState InState);
	FORCEINLINE void PopTopState(EParseState InState);
	FORCEINLINE bool IsLineBreak(const TCharType& Char);
	FORCEINLINE bool IsWhitespace(const TCharType& Char);

	bool bTopObjectAtValue = false;
	FResult EndTopRead();
};

//	actually these can be moved into .inl
FJsonReader::EParseState FJsonReader::GetTopState()
{
	return States.Top();
}

void FJsonReader::PushTopState(EParseState InState)
{
	States.Push(InState);
}

void FJsonReader::PopTopState(EParseState InState)
{
	check(GetTopState() == InState);
	States.Pop();
}

bool FJsonReader::IsLineBreak(const TCharType& Char)
{
	return Char == TCharType('\n');
}

bool FJsonReader::IsWhitespace(const TCharType& Char)
{
	return Char == TCharType(' ')
		|| Char == TCharType('\t')
		|| Char == TCharType('\n')
		|| Char == TCharType('\r');
}


} // namespace DataConfig









