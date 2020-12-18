#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Source/DcSourceTypes.h"
#include "DataConfig/Source/DcSourceUtils.h"
#include "DataConfig/Diagnostic/DcDiagnostic.h"
#include "DataConfig/Misc/DcTypeUtils.h"

template<typename CharType>
struct TDcJsonReader : public FDcReader, private FNoncopyable
{
	using SourceView = TDcSourceView<CharType>;
	using SourceRef = TDcSourceRef<CharType>;
	using SourceUtils = TDcCSourceUtils<CharType>;
	using CString = TCString<CharType>;

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

		_Count,
	};

	static EDcDataEntry TokenTypeToDataEntry(ETokenType TokenType);

	constexpr static CharType _TRUE_LITERAL[] = { 't','r','u','e',0 };
	constexpr static CharType _FALSE_LITERAL[] = { 'f','a','l','s','e',0 };
	constexpr static CharType _NULL_LITERAL[] = { 'n','u','l','l',0 };
	constexpr static CharType _EOF_CHAR = CharType('\0');
	constexpr static uint32 _MAX_KEY_LEN = 2048;

	struct FTokenFlag
	{
		uint8 bStringHasEscapeChar : 1;
		uint8 bNumberIsNegative : 1;
		uint8 bNumberHasDecimal : 1;
		uint8 bNumberHasExp : 1;

		uint8 NumberDecimalOffset;

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

	TDcJsonReader();

	void SetNewString(const CharType* InStrPtr, int32 Num);

	template<typename TArrayChar, typename TArrayAllocator>
	void SetNewString(const TArray<TArrayChar, TArrayAllocator>& InCharArr)
	{
		static_assert(DcTypeUtils::TIsSameSize<TArrayChar, CharType>::Value, "array element type isn't same size as this reader");
		SetNewString((const CharType*)(InCharArr.GetData()), InCharArr.Num());
	}

	FORCEINLINE void SetNewString(const CharType* InStrPtr) { SetNewString(InStrPtr, CString::Strlen(InStrPtr)); }


	enum class EState
	{
		Unitialized,
		InitializedWithStr,
		FinishedStr,
		Invalid,
	};
	EState State = EState::Unitialized;

	SourceView Buf = {};
	FDcSourceLocation Loc = {1, 0};

	FORCEINLINE void NewLine() { Loc.Line++; Loc.Column = 0; }

	FToken Token = {};
	FToken CachedNext;

	int32 Cur = 0;
	FString DiagFilePath = TEXT("<unknown file>");

	bool Coercion(EDcDataEntry ToEntry) override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;

	FDcResult ReadNil() override;
	FDcResult ReadBool(bool* OutPtr) override;
	FDcResult ReadName(FName* OutPtr) override;
	FDcResult ReadString(FString* OutPtr) override;
	FDcResult ReadText(FText* OutPtr) override;

	FDcResult ReadMapRoot() override;
	FDcResult ReadMapEnd() override;
	FDcResult ReadArrayRoot() override;
	FDcResult ReadArrayEnd() override;

	FDcResult ReadInt8(int8* OutPtr) override;
	FDcResult ReadInt16(int16* OutPtr) override;
	FDcResult ReadInt32(int32* OutPtr) override;
	FDcResult ReadInt64(int64* OutPtr) override;

	FDcResult ReadUInt8(uint8* OutPtr) override;
	FDcResult ReadUInt16(uint16* OutPtr) override;
	FDcResult ReadUInt32(uint32* OutPtr) override;
	FDcResult ReadUInt64(uint64* OutPtr) override;

	FDcResult ReadFloat(float* OutPtr) override;
	FDcResult ReadDouble(double* OutPtr) override;

	FDcResult ConsumeRawToken();
	FDcResult ConsumeEffectiveToken();

	void PutbackToken(const FToken& Putback);

	bool IsAtEnd(int N = 0);
	void Advance();
	void AdvanceN(int N);
	CharType PeekChar(int N = 0);

	FDcResult ReadWordExpect(const CharType* Word);

	void ReadWhiteSpace();
	void ReadLineComment();
	FDcResult ReadBlockComment();

	FDcResult ReadStringToken();
	FDcResult ParseStringToken(FString &OutStr);

	FDcResult ReadNumberToken();

	enum class EParseState
	{
		Nil,
		Object,
		Array,
	};

	TArray<EParseState, TInlineAllocator<8>> States;
	using FKeys = TArray<FName, TInlineAllocator<8>>;
	TArray<FKeys, TInlineAllocator<8>> Keys;

	FORCEINLINE EParseState GetTopState() { return States.Top(); }
	FORCEINLINE void PushTopState(EParseState InState) { States.Push(InState); }
	FORCEINLINE void PopTopState(EParseState InState) { check(GetTopState() == InState); States.Pop(); }
	FORCEINLINE bool IsAtObjectKey() { return GetTopState() == EParseState::Object && !bTopObjectAtValue; }

	bool bTopObjectAtValue;
	bool bNeedConsumeToken;

	FDcResult ReadTokenAsDataEntry(EDcDataEntry* OutPtr);
	FDcResult CheckConsumeToken(EDcDataEntry Expect);

	FDcResult EndTopRead();

	FDcDiagnosticHighlight FormatHighlight(SourceRef SpanRef);
	FORCEINLINE FDcDiagnosticHighlight FormatHighlight(int Begin, int Num) { return FormatHighlight(SourceRef{ &Buf, Begin, Num }); }

	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	template<typename TInt>
	FDcResult ParseInteger(TInt* OutPtr);
	template<typename TInt>
	FDcResult ReadSignedInteger(TInt* OutPtr);
	template<typename TInt>
	FDcResult ReadUnsignedInteger(TInt* OutPtr);
	template<typename TFloat>
	FDcResult ReadFloating(TFloat* OutPtr);

	FDcResult CheckNotObjectKey();
	FDcResult CheckObjectDuplicatedKey(const FName& KeyName);

};

extern template struct TDcJsonReader<ANSICHAR>;
extern template struct TDcJsonReader<WIDECHAR>;

using FDcJsonReader = TDcJsonReader<TCHAR>;
using FDcAnsiJsonReader = TDcJsonReader<ANSICHAR>;

