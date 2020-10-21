#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"

struct DATACONFIGCORE_API FDcJsonReader : public FDcReader, private FNoncopyable
{
	using TCharType = TCHAR;

	FDcJsonReader();
	FDcJsonReader(const FString* InStrPtr);

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

	EDcDataEntry Peek() override;

	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;

	bool IsAtEnd();
	void Advance();
	TCharType ReadChar();
	TCharType PeekChar();

	FDcResult TryPeekChar(TCharType& OutChar);
	FDcResult ReadWordExpect(const TCharType* Word);
	FDcResult ReadCharExpect(TCharType Expect);
	FDcResult ReadString(FString& OutStr);

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
	FDcResult EndTopRead();
};

//	actually these can be moved into .inl
FDcJsonReader::EParseState FDcJsonReader::GetTopState()
{
	return States.Top();
}

void FDcJsonReader::PushTopState(EParseState InState)
{
	States.Push(InState);
}

void FDcJsonReader::PopTopState(EParseState InState)
{
	check(GetTopState() == InState);
	States.Pop();
}

bool FDcJsonReader::IsLineBreak(const TCharType& Char)
{
	return Char == TCharType('\n');
}

bool FDcJsonReader::IsWhitespace(const TCharType& Char)
{
	return Char == TCharType(' ')
		|| Char == TCharType('\t')
		|| Char == TCharType('\n')
		|| Char == TCharType('\r');
}


