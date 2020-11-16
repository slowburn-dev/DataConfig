#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "Misc/Parse.h"

template<typename CharType>
EDcDataEntry TDcJsonReader<CharType>::TokenTypeToDataEntry(ETokenType TokenType)
{
	static EDcDataEntry _Mapping[ETokenType::_Count] = {
		EDcDataEntry::Ended,
		EDcDataEntry::Nil,
		EDcDataEntry::MapRoot,
		EDcDataEntry::MapEnd,
		EDcDataEntry::ArrayRoot,
		EDcDataEntry::ArrayEnd,
		EDcDataEntry::Nil,
		EDcDataEntry::String,
		EDcDataEntry::Double,
		EDcDataEntry::Bool,
		EDcDataEntry::Bool,
		EDcDataEntry::Nil,

		EDcDataEntry::Nil,
		EDcDataEntry::Nil,
		EDcDataEntry::Nil,
	};

	return _Mapping[(int)TokenType];
}

template<typename CharType>
TDcJsonReader<CharType>::TDcJsonReader()
{
	States.Add(EParseState::Nil);
}

template<typename CharType>
void TDcJsonReader<CharType>::SetNewString(const CharType* InStrPtr, int32 Num)
{
	check(State == EState::Unitialized || State == EState::FinishedStr);
	Buf = SourceView(InStrPtr, Num);

	Token.Type = ETokenType::EOF_;
	Token.Ref.Reset();
	Token.Ref.Buffer = &Buf;

	State = EState::InitializedWithStr;
	Cur = 0;
	Loc.Line = 1;
	Loc.Column = 0;
}

template<typename CharType>
bool TDcJsonReader<CharType>::Coercion(EDcDataEntry ToEntry)
{
	if (Token.Type == ETokenType::Number)
	{
		return FDcTypeUtils::IsNumericDataEntry(ToEntry)
			|| ToEntry == EDcDataEntry::String;
	}
	else if (Token.Type == ETokenType::String)
	{
		return ToEntry == EDcDataEntry::Name;
	}

	return false;
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadNext(EDcDataEntry* OutPtr)
{
	DC_TRY(ConsumeEffectiveToken());

	switch (Token.Type)
	{
	case ETokenType::True:
	case ETokenType::False:
	case ETokenType::Null:
	case ETokenType::CurlyOpen:
	case ETokenType::CurlyClose:
	case ETokenType::SquareOpen:
	case ETokenType::SquareClose:
	case ETokenType::String:
	case ETokenType::Number:
	case ETokenType::EOF_:
		{
			ReadOut(OutPtr, TokenTypeToDataEntry(Token.Type));
			return DcOk();
		}
	default:
		return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadNil()
{
	if (Token.Type == ETokenType::Null)
	{
		CheckNotObjectKey();
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Nil << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadBool(bool* OutPtr)
{
	if (Token.Type == ETokenType::True)
	{
		CheckNotObjectKey();
		ReadOut(OutPtr, true);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else if (Token.Type == ETokenType::False)
	{
		CheckNotObjectKey();
		ReadOut(OutPtr, false);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Bool << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}


template<typename CharType>
FDcResult TDcJsonReader<CharType>::CheckNotObjectKey()
{
	if (GetTopState() == EParseState::Object
		&& !bTopObjectAtValue)
		return DC_FAIL(DcDJSON, KeyMustBeString) << FormatHighlight(Token.Ref);
	else
		return DcOk();
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::CheckObjectDuplicatedKey(const FName& KeyName)
{
	check(Keys.Num() && IsAtObjectKey());
	if (Keys.Top().Contains(KeyName))
		return DC_FAIL(DcDJSON, DuplicatedKey) << FormatHighlight(Token.Ref);
	else
		Keys.Top().Add(KeyName);

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadName(FName* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		FName ParsedName(*ParsedStr);
		if (IsAtObjectKey())
			DC_TRY(CheckObjectDuplicatedKey(ParsedName));

		ReadOut(OutPtr, ParsedName);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Name << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadString(FString* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		if (IsAtObjectKey())
		{
			FName ParsedName(*ParsedStr);
			DC_TRY(CheckObjectDuplicatedKey(ParsedName));
		}

		ReadOut(OutPtr, ParsedStr);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else if (Token.Type == ETokenType::Number)
	{
		ReadOut(OutPtr, Token.Ref.ToString());
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::String << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadStringToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	Advance();
	while (true)
	{
		CharType Char = PeekChar();
		if (Char == _EOF_CHAR)
		{
			return DC_FAIL(DcDJSON, UnclosedStringLiteral) << FormatHighlight(Token.Ref.Begin, 1);
		}
		else if (Char == CharType('"'))
		{
			Advance();
			Token.Type = ETokenType::String;
			Token.Ref.Num = Cur - Token.Ref.Begin;
			return DcOk();
		}
		else if (Char == CharType('\\'))
		{
			Token.Flag.bStringHasEscapeChar = true;

			Advance();
			CharType EscapeChar = PeekChar();
			if (EscapeChar == CharType('"'))
				Advance();
		}
		else if (SourceUtils::IsControl(Char))
		{
			return DC_FAIL(DcDJSON, InvalidControlCharInString) << FormatHighlight(Cur, 1);
		}
		else
		{
			Advance();
		}
	}

	checkNoEntry();
	return DC_FAIL(DcDCommon, Unreachable);
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ParseStringToken(FString &OutStr)
{
	check(Token.Type == ETokenType::String);
	SourceRef literalRef = Token.Ref;
	literalRef.Begin += 1;
	literalRef.Num -= 2;
	if (!Token.Flag.bStringHasEscapeChar)
	{
		OutStr = literalRef.ToString();
		return DcOk();
	}
	else
	{
		// FParse::QuotedString needs string to be quoted
		FString RawStr = Token.Ref.ToString();
		int32 CharsRead = 0;
		bool bOk = FParse::QuotedString(RawStr.GetCharArray().GetData(), OutStr, &CharsRead);
		if (!bOk)
			return DC_FAIL(DcDJSON, InvalidStringEscaping) << FormatHighlight(Token.Ref.Begin, CharsRead);
		else
			return DcOk();
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadNumberToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	if (PeekChar() == CharType('-'))
		Token.Flag.bNumberIsNegative = true;

	Advance();
	while (!IsAtEnd())
	{
		CharType Char = PeekChar();
		if (Char == CharType('.'))
		{
			Token.Flag.bNumberHasDecimal = true;
			Token.Flag.NumberDecimalOffset = Cur - Token.Ref.Begin;
			Advance();
		}
		else if (Char == CharType('e') || Char == CharType('E'))
		{
			Token.Flag.bNumberHasExp = true;
			Advance();
		}
		else if (Char == CharType('-') || Char == CharType('+') || SourceUtils::IsDigit(Char))
		{
			Advance();
		}
		else
		{
			break;
		}
	}

	Token.Type = ETokenType::Number;
	Token.Ref.Num = Cur - Token.Ref.Begin;
	return DcOk();
}

template<typename CharType>
void TDcJsonReader<CharType>::ReadWhiteSpace()
{
	Token.Ref.Begin = Cur;

	while (!IsAtEnd())
	{
		CharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
		{
			NewLine();
		}

		if (!SourceUtils::IsWhitespace(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::Whitespace;
}

template<typename CharType>
void TDcJsonReader<CharType>::ReadLineComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == CharType('/'));
	check(PeekChar(1) == CharType('/'));
	AdvanceN(2);

	while (!IsAtEnd())
	{
		CharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::LineComment;
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadBlockComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == CharType('/'));
	check(PeekChar(1) == CharType('*'));
	AdvanceN(2);

	int Depth = 1;
	while (!IsAtEnd())
	{
		CharType Char0 = PeekChar(0);
		CharType Char1 = PeekChar(1);

		if (SourceUtils::IsLineBreak(Char0))
		{
			NewLine();
		}
		else if (Char0 == CharType('/') && Char1 == CharType('*'))
		{
			Depth += 1;
		}
		else if (Char0 == CharType('*') && Char1 == CharType('/'))
		{
			Depth -= 1;
			if (Depth == 0)
			{
				AdvanceN(2);
				break;
			}
		}

		Advance();
	}

	if (Depth != 0)
	{
		return DC_FAIL(DcDJSON, UnclosedBlockComment) << FormatHighlight(Token.Ref.Begin, 2);
	}
	else
	{
		Token.Ref.Num = Cur - Token.Ref.Begin;
		Token.Type = ETokenType::BlockComment;
		return DcOk();
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::EndTopRead()
{
	EParseState TopState = GetTopState();
	if (TopState == EParseState::Object)
	{
		if (!bTopObjectAtValue)
		{
			//	at key position
			DC_TRY(ConsumeEffectiveToken());

			if (Token.Type != ETokenType::Colon)
			{
				return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
			}

			bTopObjectAtValue = true;
			return DcOk();
		}
		else
		{
			//	at value position
			FToken Prev = Token;
			DC_TRY(ConsumeEffectiveToken());
			bTopObjectAtValue = false;

			//	allowing optional trailing comma
			if (Token.Type == ETokenType::Comma)
			{
				return DcOk();
			}
			else if (Token.Type == ETokenType::CurlyClose)
			{
				PutbackToken(Prev);
				return DcOk();
			}
			else
			{
				return DC_FAIL(DcDJSON, ExpectComma) << FormatHighlight(Token.Ref);
			}
		}
	}
	else if (TopState == EParseState::Array)
	{
		FToken Prev = Token;
		DC_TRY(ConsumeEffectiveToken());

		if (Token.Type == ETokenType::Comma)
		{
			return DcOk();
		}
		else if (Token.Type == ETokenType::SquareClose)
		{
			PutbackToken(Prev);
			return DcOk();
		}
		else
		{
			return DC_FAIL(DcDJSON, ExpectComma) << FormatHighlight(Token.Ref);
		}
	}
	else if (TopState == EParseState::Nil)
	{
		return DcOk();
	}
	else
	{
		checkNoEntry();
		return DC_FAIL(DcDCommon, Unreachable);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadMapRoot()
{
	if (Token.Type == ETokenType::CurlyOpen)
	{
		CheckNotObjectKey();
		PushTopState(EParseState::Object);
		bTopObjectAtValue = false;
		Keys.AddDefaulted();
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::MapRoot << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadMapEnd()
{
	if (Token.Type == ETokenType::CurlyClose)
	{
		PopTopState(EParseState::Object);
		bTopObjectAtValue = true;
		Keys.Pop();
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::MapEnd << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadArrayRoot()
{
	if (Token.Type == ETokenType::SquareOpen)
	{
		CheckNotObjectKey();
		PushTopState(EParseState::Array);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::ArrayRoot << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadArrayEnd()
{
	if (Token.Type == ETokenType::SquareClose)
	{
		PopTopState(EParseState::Array);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::ArrayEnd << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}


template<typename CharType>
struct TDcJsonReader_NumericDispatch
{
	using CString = TCString<CharType>;

	static FORCEINLINE void ParseIntDispatch(int8& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int16& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int32& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int64& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoi64(Ptr, OutEnd, 10); }

	static FORCEINLINE void ParseIntDispatch(uint8& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint16& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint32& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint64& OutValue, CharType** OutEnd, const CharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }

	static FORCEINLINE void ParseFloatDispatch(float& OutValue, const CharType* Ptr) { OutValue = CString::Atof(Ptr); }
	static FORCEINLINE void ParseFloatDispatch(double& OutValue, const CharType* Ptr) { OutValue = CString::Atod(Ptr); }

};

template<typename CharType>
template<typename TInt>
FDcResult TDcJsonReader<CharType>::ParseInteger(TInt* OutPtr)
{
	int IntOffset = Token.Flag.bNumberHasDecimal ? Token.Flag.NumberDecimalOffset : Token.Ref.Num;
	const CharType* BeginPtr = Token.Ref.GetBeginPtr();
	CharType* EndPtr = nullptr;

	TInt Value;
	TDcJsonReader_NumericDispatch<CharType>::ParseIntDispatch(Value, &EndPtr, BeginPtr);
	if (EndPtr - BeginPtr != IntOffset)
		return DC_FAIL(DcDJSON, ParseIntegerFailed) << FormatHighlight(Token.Ref);

	ReadOut(OutPtr, Value);
	DC_TRY(EndTopRead());
	return DcOk();
}

template<typename CharType>
template<typename TInt>
FDcResult TDcJsonReader<CharType>::ReadSignedInteger(TInt* OutPtr)
{
	if (Token.Type == ETokenType::Number)
	{
		CheckNotObjectKey();
		return ParseInteger<TInt>(OutPtr);
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< TDcDataEntryType<TInt>::Value << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
template<typename TInt>
FDcResult TDcJsonReader<CharType>::ReadUnsignedInteger(TInt* OutPtr)
{
	if (Token.Type == ETokenType::Number)
	{
		CheckNotObjectKey();

		if (Token.Flag.bNumberIsNegative)
			return DC_FAIL(DcDJSON, ReadUnsignedWithNegativeNumber)
				<< FormatHighlight(Token.Ref);

		return ParseInteger<TInt>(OutPtr);
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< TDcDataEntryType<TInt>::Value << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}


template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt8(int8* OutPtr) { return ReadSignedInteger<int8>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt16(int16* OutPtr) { return ReadSignedInteger<int16>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt32(int32* OutPtr) { return ReadSignedInteger<int32>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt64(int64* OutPtr) { return ReadSignedInteger<int64>(OutPtr); }

template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt8(uint8* OutPtr) { return ReadUnsignedInteger<uint8>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt16(uint16* OutPtr) { return ReadUnsignedInteger<uint16>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt32(uint32* OutPtr) { return ReadUnsignedInteger<uint32>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt64(uint64* OutPtr) { return ReadUnsignedInteger<uint64>(OutPtr); }

template<typename CharType>
template<typename TFloat>
FDcResult TDcJsonReader<CharType>::ReadFloating(TFloat* OutPtr)
{
	if (Token.Type == ETokenType::Number)
	{
		CheckNotObjectKey();

		TFloat Value;
		TDcJsonReader_NumericDispatch<CharType>::ParseFloatDispatch(Value, Token.Ref.GetBeginPtr());

		ReadOut(OutPtr, Value);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< TDcDataEntryType<TFloat>::Value << TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadFloat(float* OutPtr) { return ReadFloating<float>(OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadDouble(double* OutPtr) { return ReadFloating<double>(OutPtr); }

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ConsumeRawToken()
{
	check(State == EState::InitializedWithStr);
	if (CachedNext.IsValid())
	{
		Token = CachedNext;
		CachedNext.Reset();
		return DcOk();
	}

	if (IsAtEnd())
	{
		Token.Type = ETokenType::EOF_;
		Token.Ref.Reset();
		return DcOk();
	}

	auto _ConsumeSingleCharToken = [this](ETokenType TokenType) {
		Token.Type = TokenType;
		Token.Ref.Begin = Cur;
		Token.Ref.Num = 1;
		Advance();
		return DcOk();
	};

	CharType Char = PeekChar();
	if (SourceUtils::IsWhitespace(Char))
	{
		ReadWhiteSpace();
		return DcOk();
	}
	else if (Char == CharType('/'))
	{
		CharType NextChar = PeekChar(1);
		if (NextChar == CharType('/'))
		{
			ReadLineComment();
			return DcOk();
		}
		else if (NextChar == CharType('*'))
		{
			return ReadBlockComment();
		}
		else
		{
			return DC_FAIL(DcDJSON, UnexpectedChar) << Char << FormatHighlight(Cur, 1);
		}
	}
	else if (Char == CharType('{'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyOpen);
	}
	else if (Char == CharType('}'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyClose);
	}
	else if (Char == CharType('['))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareOpen);
	}
	else if (Char == CharType(']'))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareClose);
	}
	else if (Char == CharType(':'))
	{
		return _ConsumeSingleCharToken(ETokenType::Colon);
	}
	else if (Char == CharType(','))
	{
		return _ConsumeSingleCharToken(ETokenType::Comma);
	}
	else if (Char == CharType('t'))
	{
		DC_TRY(ReadWordExpect(_TRUE_LITERAL));
		Token.Type = ETokenType::True;
		return DcOk();
	}
	else if (Char == CharType('f'))
	{
		DC_TRY(ReadWordExpect(_FALSE_LITERAL));
		Token.Type = ETokenType::False;
		return DcOk();
	}
	else if (Char == CharType('n'))
	{
		DC_TRY(ReadWordExpect(_NULL_LITERAL));
		Token.Type = ETokenType::Null;
		return DcOk();
	}
	else if (Char == CharType('"'))
	{
		return ReadStringToken();
	}
	else if (Char == CharType('-')
		|| SourceUtils::IsDigit(Char))
	{
		return ReadNumberToken();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedChar)
			<< Char << FormatHighlight(Cur, 1);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ConsumeEffectiveToken()
{
	while (true)
	{
		FDcResult Ret = ConsumeRawToken();
		if (!Ret.Ok())
			return Ret;
		else if (Token.Type < ETokenType::LineComment)
			return DcOk();
		else
			continue;
	}

	checkNoEntry();
	return DC_FAIL(DcDCommon, Unreachable);
}

template<typename CharType>
void TDcJsonReader<CharType>::PutbackToken(const FToken& Putback)
{
	check(Token.IsValid() && Putback.IsValid());
	check(!CachedNext.IsValid());
	CachedNext = Token;
	Token = Putback;
}

template<typename CharType>
bool TDcJsonReader<CharType>::IsAtEnd(int N)
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur + N >= Buf.Num;
}

template<typename CharType>
void TDcJsonReader<CharType>::Advance()
{
	check(!IsAtEnd());
	++Cur;
	Loc.Column++;
}

template<typename CharType>
void TDcJsonReader<CharType>::AdvanceN(int N)
{
	check(N != 0);
	check(!IsAtEnd(N));
	Cur += N;
	Loc.Column += N;
}

template<typename CharType>
CharType TDcJsonReader<CharType>::PeekChar(int N)
{
	if (IsAtEnd(N))
		return _EOF_CHAR;
	else
		return Buf.Buffer[Cur + N];
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadWordExpect(const CharType* Word)
{
	int32 RefBegin = Cur;
	int32 WordLen = CString::Strlen(Word);
	SourceRef WordRef = Token.Ref;
	WordRef.Begin = Cur;
	WordRef.Num = WordLen;

	if (IsAtEnd(WordLen))
	{
		return DcFail();
	}

	for (int Ix = 0; Ix < WordLen; Ix++)
	{
		if (CharType(Word[Ix]) != PeekChar(Ix))
		{
			return DC_FAIL(DcDJSON, ExpectWordButNotFound)
				<< Word << WordRef.ToString()
				<< FormatHighlight(WordRef);
		}
	}

	Token.Ref = WordRef;
	AdvanceN(WordLen);
	return DcOk();
}

template<typename CharType>
FDcDiagnosticHighlightWithFileContext TDcJsonReader<CharType>::FormatHighlight(SourceRef SpanRef)
{
	FDcDiagnosticHighlightWithFileContext OutHighlight;
	OutHighlight.FileContext.Loc = Loc;
	OutHighlight.FileContext.FilePath = DiagFilePath;
	THightlightFormatter<CharType> Highlighter;
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, Loc);

	return OutHighlight;
}

template struct DATACONFIGCORE_API TDcJsonReader<ANSICHAR>;
template struct DATACONFIGCORE_API TDcJsonReader<WIDECHAR>;

