#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "Misc/Parse.h"


EDcDataEntry FDcJsonReader::TokenTypeToDataEntry(ETokenType TokenType)
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

FDcJsonReader::FDcJsonReader(const FString* InStrPtr)
	: FDcJsonReader()
{
	SetNewString(InStrPtr);
}

FDcJsonReader::FDcJsonReader()
{
	States.Add(EParseState::Nil);
}

void FDcJsonReader::SetNewString(const FString* InStrPtr)
{
	check(State == EState::Unitialized || State == EState::FinishedStr);
	Buf = SourceBuf(InStrPtr->GetCharArray().GetData());
	Token.Type = ETokenType::EOF_;
	Token.Ref.Reset();
	Token.Ref.Buffer = &Buf;

	State = EState::InitializedWithStr;
	Cur = 0;
	Loc.Line = 0;
	Loc.Column = 0;
}

bool FDcJsonReader::Coercion(EDcDataEntry ToEntry)
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

FDcResult FDcJsonReader::ReadNext(EDcDataEntry* OutPtr)
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

FDcResult FDcJsonReader::ReadNil()
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

FDcResult FDcJsonReader::ReadBool(bool* OutPtr)
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


FDcResult FDcJsonReader::CheckNotObjectKey()
{
	if (GetTopState() == EParseState::Object
		&& !bTopObjectAtValue)
		return DC_FAIL(DcDJSON, KeyMustBeString) << FormatHighlight(Token.Ref);
	else
		return DcOk();
}


FDcResult FDcJsonReader::CheckObjectDuplicatedKey(const FName& KeyName)
{
	if (GetTopState() == EParseState::Object
		&& !bTopObjectAtValue)
	{
		check(Keys.Num());
		if (Keys.Top().Contains(KeyName))
			return DC_FAIL(DcDJSON, DuplicatedKey) << FormatHighlight(Token.Ref);
		else
			Keys.Top().Add(KeyName);
	}

	return DcOk();
}

FDcResult FDcJsonReader::ReadName(FName* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		FName ParsedName(*ParsedStr);
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

FDcResult FDcJsonReader::ReadString(FString* OutPtr)
{
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		DC_TRY(CheckObjectDuplicatedKey(*ParsedStr));

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

FDcResult FDcJsonReader::ReadStringToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	Advance();
	while (true)
	{
		TCharType Char = PeekChar();
		if (Char == _EOF_CHAR)
		{
			return DC_FAIL(DcDJSON, UnclosedStringLiteral) << FormatHighlight(Token.Ref.Begin, 1);
		}
		else if (Char == TCharType('"'))
		{
			Advance();
			Token.Type = ETokenType::String;
			Token.Ref.Num = Cur - Token.Ref.Begin;
			return DcOk();
		}
		else if (Char == TCharType('\\'))
		{
			Token.Flag.bStringHasEscapeChar = true;

			Advance();
			TCharType EscapeChar = PeekChar();
			if (EscapeChar == TCharType('"'))
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

FDcResult FDcJsonReader::ParseStringToken(FString &OutStr)
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

FDcResult FDcJsonReader::ReadNumberToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	if (PeekChar() == TCharType('-'))
		Token.Flag.bNumberIsNegative = true;

	Advance();
	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (Char == TCharType('.'))
		{
			Token.Flag.bNumberHasDecimal = true;
			Token.Flag.NumberDecimalOffset = Cur - Token.Ref.Begin;
			Advance();
		}
		else if (Char == TCharType('e') || Char == TCharType('E'))
		{
			Token.Flag.bNumberHasExp = true;
			Advance();
		}
		else if (Char == TCharType('-') || Char == TCharType('+') || SourceUtils::IsDigit(Char))
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

void FDcJsonReader::ReadWhiteSpace()
{
	Token.Ref.Begin = Cur;

	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
		{
			Loc.Line++;
			Loc.Column = 0;
		}

		if (!SourceUtils::IsWhitespace(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::Whitespace;
}

void FDcJsonReader::ReadLineComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == TCharType('/'));
	check(PeekChar(1) == TCharType('/'));
	AdvanceN(2);

	while (!IsAtEnd())
	{
		TCharType Char = PeekChar();
		if (SourceUtils::IsLineBreak(Char))
			break;

		Advance();
	}

	Token.Ref.Num = Cur - Token.Ref.Begin;
	Token.Type = ETokenType::LineComment;
}

FDcResult FDcJsonReader::ReadBlockComment()
{
	Token.Ref.Begin = Cur;
	check(PeekChar(0) == TCharType('/'));
	check(PeekChar(1) == TCharType('*'));
	AdvanceN(2);

	int Depth = 1;
	while (!IsAtEnd())
	{
		TCharType Char0 = PeekChar(0);
		TCharType Char1 = PeekChar(1);

		if (Char0 == TCharType('/') && Char1 == TCharType('*'))
		{
			Depth += 1;
		}
		else if (Char0 == TCharType('*') && Char1 == TCharType('/'))
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

FDcResult FDcJsonReader::EndTopRead()
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
				return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
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
			return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
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

FDcResult FDcJsonReader::ReadMapRoot()
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

FDcResult FDcJsonReader::ReadMapEnd()
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

FDcResult FDcJsonReader::ReadArrayRoot()
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

FDcResult FDcJsonReader::ReadArrayEnd()
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


template<typename TCharType>
struct TDcJsonReader_NumericDispatch
{
	using CString = TCString<TCharType>;

	static FORCEINLINE void ParseIntDispatch(int8& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int16& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int32& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoi(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(int64& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoi64(Ptr, OutEnd, 10); }

	static FORCEINLINE void ParseIntDispatch(uint8& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint16& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint32& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }
	static FORCEINLINE void ParseIntDispatch(uint64& OutValue, TCharType** OutEnd, const TCharType* Ptr) { OutValue = CString::Strtoui64(Ptr, OutEnd, 10); }

	static FORCEINLINE void ParseFloatDispatch(float& OutValue, const TCharType* Ptr) { OutValue = CString::Atof(Ptr); }
	static FORCEINLINE void ParseFloatDispatch(double& OutValue, const TCharType* Ptr) { OutValue = CString::Atod(Ptr); }

};

template<typename TInt>
FDcResult FDcJsonReader::ParseInteger(TInt* OutPtr)
{
	int IntOffset = Token.Flag.bNumberHasDecimal ? Token.Flag.NumberDecimalOffset : Token.Ref.Num;
	const TCharType* BeginPtr = Token.Ref.GetBeginPtr();
	TCharType* EndPtr = nullptr;

	TInt Value;
	TDcJsonReader_NumericDispatch<TCharType>::ParseIntDispatch(Value, &EndPtr, BeginPtr);
	if (EndPtr - BeginPtr != IntOffset)
		return DC_FAIL(DcDJSON, ParseIntegerFailed) << FormatHighlight(Token.Ref);

	ReadOut(OutPtr, Value);
	DC_TRY(EndTopRead());
	return DcOk();
}

template<typename TInt>
FDcResult FDcJsonReader::ReadSignedInteger(TInt* OutPtr)
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

template<typename TInt>
FDcResult FDcJsonReader::ReadUnsignedInteger(TInt* OutPtr)
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


FDcResult FDcJsonReader::ReadInt8(int8* OutPtr) { return ReadSignedInteger<int8>(OutPtr); }
FDcResult FDcJsonReader::ReadInt16(int16* OutPtr) { return ReadSignedInteger<int16>(OutPtr); }
FDcResult FDcJsonReader::ReadInt32(int32* OutPtr) { return ReadSignedInteger<int32>(OutPtr); }
FDcResult FDcJsonReader::ReadInt64(int64* OutPtr) { return ReadSignedInteger<int64>(OutPtr); }

FDcResult FDcJsonReader::ReadUInt8(uint8* OutPtr) { return ReadUnsignedInteger<uint8>(OutPtr); }
FDcResult FDcJsonReader::ReadUInt16(uint16* OutPtr) { return ReadUnsignedInteger<uint16>(OutPtr); }
FDcResult FDcJsonReader::ReadUInt32(uint32* OutPtr) { return ReadUnsignedInteger<uint32>(OutPtr); }
FDcResult FDcJsonReader::ReadUInt64(uint64* OutPtr) { return ReadUnsignedInteger<uint64>(OutPtr); }

template<typename TFloat>
FDcResult FDcJsonReader::ReadFloating(TFloat* OutPtr)
{
	if (Token.Type == ETokenType::Number)
	{
		CheckNotObjectKey();

		TFloat Value;
		TDcJsonReader_NumericDispatch<TCharType>::ParseFloatDispatch(Value, Token.Ref.GetBeginPtr());

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

FDcResult FDcJsonReader::ReadFloat(float* OutPtr) { return ReadFloating<float>(OutPtr); }
FDcResult FDcJsonReader::ReadDouble(double* OutPtr) { return ReadFloating<double>(OutPtr); }

FDcResult FDcJsonReader::ConsumeRawToken()
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

	TCharType Char = PeekChar();
	if (SourceUtils::IsWhitespace(Char))
	{
		ReadWhiteSpace();
		return DcOk();
	}
	else if (Char == TCharType('/'))
	{
		TCharType NextChar = PeekChar(1);
		if (NextChar == TCharType('/'))
		{
			ReadLineComment();
			return DcOk();
		}
		else if (NextChar == TCharType('*'))
		{
			return ReadBlockComment();
		}
		else
		{
			return DC_FAIL(DcDJSON, UnexpectedChar) << Char << FormatHighlight(Cur, 1);
		}
	}
	else if (Char == TCharType('{'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyOpen);
	}
	else if (Char == TCharType('}'))
	{
		return _ConsumeSingleCharToken(ETokenType::CurlyClose);
	}
	else if (Char == TCharType('['))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareOpen);
	}
	else if (Char == TCharType(']'))
	{
		return _ConsumeSingleCharToken(ETokenType::SquareClose);
	}
	else if (Char == TCharType(':'))
	{
		return _ConsumeSingleCharToken(ETokenType::Colon);
	}
	else if (Char == TCharType(','))
	{
		return _ConsumeSingleCharToken(ETokenType::Comma);
	}
	else if (Char == TCharType('t'))
	{
		DC_TRY(ReadWordExpect(_TRUE_LITERAL));
		Token.Type = ETokenType::True;
		return DcOk();
	}
	else if (Char == TCharType('f'))
	{
		DC_TRY(ReadWordExpect(_FALSE_LITERAL));
		Token.Type = ETokenType::False;
		return DcOk();
	}
	else if (Char == TCharType('n'))
	{
		DC_TRY(ReadWordExpect(_NULL_LITERAL));
		Token.Type = ETokenType::Null;
		return DcOk();
	}
	else if (Char == TCharType('"'))
	{
		return ReadStringToken();
	}
	else if (Char == TCharType('-')
		|| SourceUtils::IsDigit(Char))
	{
		return ReadNumberToken();
	}
	else
	{
		return DC_FAIL(DcDJSON, UnexpectedChar)
			<< FString::Chr(Char) << FormatHighlight(Cur, 1);
	}
}

FDcResult FDcJsonReader::ConsumeEffectiveToken()
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

void FDcJsonReader::PutbackToken(const FToken& Putback)
{
	check(Token.IsValid() && Putback.IsValid());
	check(!CachedNext.IsValid());
	CachedNext = Token;
	Token = Putback;
}

bool FDcJsonReader::IsAtEnd(int N)
{
	check(State != EState::Unitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur + N >= Buf.Num;
}

void FDcJsonReader::Advance()
{
	check(!IsAtEnd());
	++Cur;
	Loc.Column++;
}

void FDcJsonReader::AdvanceN(int N)
{
	check(N != 0);
	check(!IsAtEnd(N));
	Cur += N;
	Loc.Column += N;
}

FDcJsonReader::TCharType FDcJsonReader::PeekChar(int N)
{
	if (IsAtEnd(N))
		return _EOF_CHAR;
	else
		return Buf.Buffer[Cur + N];
}

FDcResult FDcJsonReader::ReadWordExpect(const TCharType* Word)
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
		if (TCharType(Word[Ix]) != PeekChar(Ix))
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


FDcDiagnosticHighlight FDcJsonReader::FormatHighlight(SourceRef SpanRef)
{
	FDcDiagnosticHighlight OutHighlight;
	OutHighlight.Loc = Loc;
	OutHighlight.FilePath = DiagFilePath;
	FHightlightFormatter<TCharType> Highlighter;
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, Loc);

	return OutHighlight;
}

FDcDiagnosticHighlight FDcJsonReader::FormatHighlight(int Begin, int Num)
{
	return FormatHighlight(SourceRef{ &Buf, Begin, Num });
}

