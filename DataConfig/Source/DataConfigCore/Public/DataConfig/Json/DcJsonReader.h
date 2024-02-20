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
	using TSelf = TDcJsonReader;

	using SourceView = TDcSourceView<CharType>;
	using SourceRef = TDcSourceRef<CharType>;
	using SourceUtils = TDcCSourceUtils<CharType>;
	using CString = TCString<CharType>;

	enum class ETokenType : uint8
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

	struct FTokenFlag
	{
		uint8 bStringHasEscapeChar : 1;
		uint8 bStringHasNonAscii : 1;
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
	TDcJsonReader(const CharType* Str);
	TDcJsonReader(const CharType* Buf, int Len);

	void AbortAndUninitialize();
	FDcResult FinishRead();
	FDcResult SetNewString(const CharType* InStrPtr, int32 Num);

	template<typename TArrayChar, typename TArrayAllocator>
	FDcResult SetNewString(const TArray<TArrayChar, TArrayAllocator>& InCharArr)
	{
		static_assert(DcTypeUtils::TIsSameSize<TArrayChar, CharType>::Value, "array element type isn't same size as this reader");
		return SetNewString((const CharType*)(InCharArr.GetData()), InCharArr.Num());
	}

	FORCEINLINE FDcResult SetNewString(const CharType* InStrPtr) { return SetNewString(InStrPtr, CString::Strlen(InStrPtr)); }


	enum class EState : uint8
	{
		Uninitialized,
		InProgress,
		FinishedStr,
		Invalid,
	};
	EState State = EState::Uninitialized;

	SourceView Buf = {};
	FDcSourceLocation Loc = {1, 0};

	FORCEINLINE void NewLine() { Loc.Line++; Loc.Column = 0; }

	FToken Token = {};
	FToken CachedNext;

	int32 Cur = 0;
	FString DiagFilePath;

	FDcResult Coercion(EDcDataEntry ToEntry, bool* OutPtr) override;
	FDcResult PeekRead(EDcDataEntry* OutPtr) override;

	FDcResult ReadNone() override;
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

	FDcResult ReadWordExpect(const CharType* Word, int32 WordLen);

	void ReadWhiteSpace();
	void ReadLineComment();
	FDcResult ReadBlockComment();

	FDcResult ReadStringToken();
	FDcResult ParseStringToken(FString &OutStr);

	FDcResult ReadNumberToken();

	enum class EParseState : uint8
	{
		Root,
		Object,
		Array,
	};

	TArray<EParseState, TInlineAllocator<8>> States;
	using FKeys = TArray<FString, TInlineAllocator<8>>;
	TArray<FKeys, TInlineAllocator<8>> Keys;

	FORCEINLINE EParseState GetTopState() { return States.Top(); }
	FORCEINLINE void PushTopState(EParseState InState) { States.Push(InState); }
	FORCEINLINE void PopTopState(EParseState InState) { check(GetTopState() == InState); States.Pop(); }
	FORCEINLINE bool IsAtObjectKey() { return GetTopState() == EParseState::Object && !bTopObjectAtValue; }

	bool bTopObjectAtValue = false;
	bool bNeedConsumeToken = false;

	FDcResult ReadTokenAsDataEntry(EDcDataEntry* OutPtr);
	FDcResult CheckConsumeToken(EDcDataEntry Expect);

	FDcResult EndTopRead();

	FDcDiagnosticHighlight FormatHighlight(SourceRef SpanRef);
	FORCEINLINE FDcDiagnosticHighlight FormatHighlight(int Begin, int Num) { return FormatHighlight(SourceRef{ &Buf, Begin, Num }); }

	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	FDcResult CheckNotObjectKey();
	FDcResult CheckObjectDuplicatedKey(const FString& Key);
	FDcResult CheckNotAtEnd();

	FString ConvertStringTokenToLiteral(SourceRef Ref);

	static FName ClassId();
	FName GetId() override;
};

struct FDcJsonReader : public TDcJsonReader<TCHAR>
{
	using Super = TDcJsonReader;

	FDcJsonReader() : Super() {}
	FDcJsonReader(const TCHAR* Str) : Super(Str) {}
	FDcJsonReader(const FString& Str) : Super(*Str) {}
};

using FDcWideJsonReader = TDcJsonReader<WIDECHAR>;
using FDcAnsiJsonReader = TDcJsonReader<ANSICHAR>;
