#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Source/DcSourceTypes.h"
#include "DataConfig/Source/DcSourceUtils.h"

struct DATACONFIGCORE_API FDcJsonReader : public FDcReader, private FNoncopyable
{
	using TCharType = TCHAR;
	using SourceBuf = TDcSourceBuffer<TCharType>;
	using SourceRef = TDcSourceRef<TCharType>;
	using SourceUtils = TDcCSourceUtils<TCharType>;
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

		LineComment,	//	`// Foo`
		BlockComment,	//	`/* Foo */`
		Whitespace,		// \r\t and space
	};

	constexpr static TCharType _TRUE_LITERAL[] = { 't','r','u','e',0 };
	constexpr static TCharType _FALSE_LITERAL[] = { 'f','a','l','s','e',0 };
	constexpr static TCharType _NULL_LITERAL[] = { 'n','u','l','l',0 };
	constexpr static TCharType _EOF_CHAR = TCharType('\0');

	struct FTokenFlag
	{
		bool bStringHasEscapeChar : 1;
		bool bNumberHasDecimal : 1;
		bool bNumberHasExp : 1;

		FORCEINLINE void Reset() { *this = FTokenFlag{}; }
	};

	struct FToken
	{
		ETokenType Type;
		SourceRef Ref;
		FTokenFlag Flag;

		FORCEINLINE bool IsValid() const { return Ref.IsValid(); }
		FORCEINLINE void Reset() { Ref.Reset(); }
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
	FDcSourceLocation Loc = {1, 0};

	//	current peeking ahead token
	FToken Token;
	FToken CachedNext;

	int32 Cur = 0;
	FString DiagFilePath = TEXT("<unknown file>");

	bool Coercion(EDcDataEntry ToEntry) override;
	FDcResult ReadNext(EDcDataEntry* OutPtr) override;

	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;

	FDcResult ReadInt32(int32* OutPtr);
	FDcResult ReadUInt32(uint32* OutPtr);
	FDcResult ReadDouble(double* OutPtr) override;

	FDcResult ConsumeRawToken();
	FDcResult ConsumeEffectiveToken();

	void PutbackToken(const FToken& Putback);

	bool IsAtEnd(int N = 0);
	void Advance();
	void AdvanceN(int N);
	TCharType PeekChar(int N = 0);

	FDcResult ReadWordExpect(const TCharType* Word);


	void ReadWhiteSpace();
	void ReadLineComment();
	FDcResult ReadBlockComment();

	FDcResult ReadStringToken();
	FDcResult ParseStringToken(FString &OutStr);

	FDcResult ReadNumberToken();

	template<typename TInt>
	FDcResult ParseInteger(TInt& OutInt);

	enum class EParseState
	{
		Nil,
		Object,
		Array,
	};

	TArray<EParseState, TInlineAllocator<8>> States;
	FORCEINLINE EParseState GetTopState() { return States.Top(); }
	FORCEINLINE void PushTopState(EParseState InState) { States.Push(InState); }
	FORCEINLINE void PopTopState(EParseState InState) { check(GetTopState() == InState); States.Pop(); }

	bool bTopObjectAtValue = false;
	FDcResult EndTopRead();

	FDcDiagnosticHighlight FormatInputSpan(SourceRef SpanRef);
	FDcDiagnosticHighlight FormatInputSpan(int Begin, int Num);
};

