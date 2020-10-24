#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Source/DcSourceTypes.h"

struct DATACONFIGCORE_API FDcJsonReader : public FDcReader, private FNoncopyable
{
	using TCharType = TCHAR;
	using SourceBuf = TDcSourceBuffer<TCharType>;
	using SourceRef = TDcSourceRef<TCharType>;
	using CString = TCString<TCharType>;

	enum class ETokenType
	{
		EOF_,
		Comma,			// ,
		CurlyOpen,		// {
		CurlyClose,		// }
		SquareOpen,		// [
		SquareClose,	// ]
		Colon,			// :
		String,			// ""
		Number,			// 12.3
		True,			// true
		False,			// false
		Null,			// null
	};

	constexpr static TCharType _TRUE_LITERAL[] = { 't','r','u','e',0 };
	constexpr static TCharType _FALSE_LITERAL[] = { 'f','a','l','s','e',0 };
	constexpr static TCharType _NULL_LITERAL[] = { 'n','u','l','l',0 };

	struct FToken
	{
		ETokenType Type;
		SourceRef Ref;
	};

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

	SourceBuf Buf = {};
	FDcSourceLocation Loc = {};

	//	current peeking ahead token
	FToken Token;

	int32 Cur = 0;
	int32 LineStart = 0;

	EDcDataEntry Peek() override;

	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;

	//	parsing functions
	FDcResult ConsumeToken();

	bool IsAtEnd(int N = 0);
	void Advance();
	void AdvanceN(int N);
	TCharType ReadChar();
	TCharType PeekChar(int N = 0);

	FDcResult TryPeekChar(TCharType& OutChar);
	FDcResult ReadWordExpect(const TCharType* Word);
	FDcResult ReadCharExpect(TCharType Expect);
	FDcResult ReadString(FString& OutStr);

	FDcResult ReadStringToken();

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
	FORCEINLINE static bool IsLineBreak(const TCharType& Char);
	FORCEINLINE static bool IsWhitespace(const TCharType& Char);

	bool bTopObjectAtValue = false;
	FDcResult EndTopRead();

	FString FormatAtCurrentLoc(int32);
	FDcInputSpan FormatInputSpan();
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


