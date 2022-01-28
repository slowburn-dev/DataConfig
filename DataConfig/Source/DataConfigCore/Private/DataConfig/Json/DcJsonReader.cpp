#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTypeUtils.h"

namespace DcJsonReaderDetails
{

template<typename CharType>
struct TNumericDispatch
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
struct TJsonReaderClassIdSelector;
template<> struct TJsonReaderClassIdSelector<ANSICHAR> { static constexpr const TCHAR* Id = TEXT("AnsiCharDcJsonReader"); };
template<> struct TJsonReaderClassIdSelector<WIDECHAR> { static constexpr const TCHAR* Id = TEXT("WideCharDcJsonReader"); };

} // namespace DcJsonReaderDetails


template<typename CharType>
struct FDcJsonReaderDetails
{

using TSelf = TDcJsonReader<CharType>;
using ETokenType = typename TSelf::ETokenType;

static FDcResult ParseQuotedString(TSelf* Self, const FString& InStr, FString& OutStr)
{
	OutStr.Reserve(InStr.Len() + 1);

	int _StrIx = 0;
	auto _GetCh = [&InStr, &_StrIx]()
	{
		return _StrIx < InStr.Len()
			? InStr[_StrIx++]
			: '\0';
	};

	auto _Fail = [Self, _StrIx]()
	{
		return DC_FAIL(DcDJSON, InvalidStringEscaping)
			<< Self->FormatHighlight(Self->Token.Ref.Begin, _StrIx);
	};

	bool bHasUnicodeEscapes = false;
	while (true)
	{
		TCHAR Ch = _GetCh();
		if (Ch == '\0')
			break;

		switch (Ch)
		{
			case '\\':
			{
				switch (_GetCh())
				{
					case '\"': OutStr.AppendChar('"'); break;
					case '\\': OutStr.AppendChar('\\'); break;
					case '/': OutStr.AppendChar('/'); break;
					case 'b': OutStr.AppendChar('\b'); break;
					case 'f': OutStr.AppendChar('\f'); break;
					case 'n': OutStr.AppendChar('\n'); break;
					case 'r': OutStr.AppendChar('\r'); break;
					case 't': OutStr.AppendChar('\t'); break;

					case 'u':
					{
						bHasUnicodeEscapes = true;
						int CodePoint = 0;
						for (int Ix = 0; Ix < 4; Ix++)
						{
							TCHAR Hex = _GetCh();
							if (Hex == '\0'
								|| !TSelf::SourceUtils::IsHexDigit(Hex))
								return _Fail();

							CodePoint += FParse::HexDigit(Hex) << ((3 - Ix) * 4);
						}

						OutStr.AppendChar(TCHAR(CodePoint));
						break;
					}

					default:
						return _Fail();
				}
				break;
			}

			default:
			{
				OutStr.AppendChar(Ch);
				break;
			}
		}
	}

	if (bHasUnicodeEscapes)
		StringConv::InlineCombineSurrogates(OutStr);
	
	return DcOk();
}

static FORCEINLINE EDcDataEntry TokenTypeToDataEntry(ETokenType TokenType)
{
	static EDcDataEntry _Mapping[(int)ETokenType::_Count] = {
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

static bool CheckCoercionRule(TSelf* Self, EDcDataEntry ToEntry)
{
	if (Self->Token.Type == ETokenType::Number)
	{
		return DcTypeUtils::IsNumericDataEntry(ToEntry)
			|| ToEntry == EDcDataEntry::String;
	}
	else if (Self->Token.Type == ETokenType::String)
	{
		return ToEntry == EDcDataEntry::Name
			|| ToEntry == EDcDataEntry::Text;
	}

	return false;
}

template<typename TInt>
static FDcResult ParseInteger(TSelf* Self, TInt* OutPtr)
{
	int IntOffset = Self->Token.Flag.bNumberHasDecimal
		? Self->Token.Flag.NumberDecimalOffset
		: Self->Token.Ref.Num;
	const CharType* BeginPtr = Self->Token.Ref.GetBeginPtr();
	CharType* EndPtr = nullptr;

	TInt Value;
	DcJsonReaderDetails::TNumericDispatch<CharType>::ParseIntDispatch(Value, &EndPtr, BeginPtr);
	if (EndPtr - BeginPtr != IntOffset)
		return DC_FAIL(DcDJSON, ParseIntegerFailed) << Self->FormatHighlight(Self->Token.Ref);

	ReadOut(OutPtr, Value);
	DC_TRY(Self->EndTopRead());
	return DcOk();
}

template<typename TInt>
static FDcResult ReadSignedInteger(TSelf* Self, TInt* OutPtr)
{

	DC_TRY(Self->CheckConsumeToken(DcTypeUtils::TDcDataEntryType<TInt>::Value));
	if (Self->Token.Type == ETokenType::Number)
	{
		DC_TRY(Self->CheckNotObjectKey());
		return ParseInteger<TInt>(Self, OutPtr);
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< DcTypeUtils::TDcDataEntryType<TInt>::Value << TokenTypeToDataEntry(Self->Token.Type)
			<< Self->FormatHighlight(Self->Token.Ref);
	}
}

template<typename TInt>
static FDcResult ReadUnsignedInteger(TSelf* Self, TInt* OutPtr)
{
	DC_TRY(Self->CheckConsumeToken(DcTypeUtils::TDcDataEntryType<TInt>::Value));
	if (Self->Token.Type == ETokenType::Number)
	{
		DC_TRY(Self->CheckNotObjectKey());

		if (Self->Token.Flag.bNumberIsNegative)
			return DC_FAIL(DcDJSON, ReadUnsignedWithNegativeNumber)
				<< Self->FormatHighlight(Self->Token.Ref);

		return ParseInteger<TInt>(Self, OutPtr);
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< DcTypeUtils::TDcDataEntryType<TInt>::Value << TokenTypeToDataEntry(Self->Token.Type)
			<< Self->FormatHighlight(Self->Token.Ref);
	}
}

template<typename TFloat>
static FDcResult ReadFloating(TSelf* Self, TFloat* OutPtr)
{
	DC_TRY(Self->CheckConsumeToken(DcTypeUtils::TDcDataEntryType<TFloat>::Value));
	if (Self->Token.Type == ETokenType::Number)
	{
		DC_TRY(Self->CheckNotObjectKey());

		TFloat Value;
		DcJsonReaderDetails::TNumericDispatch<CharType>::ParseFloatDispatch(Value, Self->Token.Ref.GetBeginPtr());

		ReadOut(OutPtr, Value);
		DC_TRY(Self->EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< DcTypeUtils::TDcDataEntryType<TFloat>::Value << TokenTypeToDataEntry(Self->Token.Type)
			<< Self->FormatHighlight(Self->Token.Ref);
	}
}

static void Reset(TSelf* Self, const CharType* InStrPtr, int32 Num)
{
	Self->Buf = typename TSelf::SourceView(InStrPtr, Num);

	Self->Token.Type = ETokenType::EOF_;
	Self->Token.Ref.Reset();
	Self->Token.Ref.Buffer = &Self->Buf;

	Self->CachedNext.Reset();
	Self->DiagFilePath.Empty();

	Self->State = TSelf::EState::InProgress;
	Self->bTopObjectAtValue = false;
	Self->bNeedConsumeToken = true;

	Self->Cur = 0;
	Self->Loc.Line = 1;
	Self->Loc.Column = 0;

	//	these are cleared by proper reads or `Abort()` should clear these on error
	check(Self->Keys.Num() == 0);
	check(Self->States.Num() == 1);
}

}; // struct FDcJsonReaderDetails

template<typename CharType>
TDcJsonReader<CharType>::TDcJsonReader()
{
	States.Add(EParseState::Root);
}

template <typename CharType>
TDcJsonReader<CharType>::TDcJsonReader(const CharType* Str)
	: TDcJsonReader()	
{
	FDcJsonReaderDetails<CharType>::Reset(this, Str, CString::Strlen(Str));
}

template <typename CharType>
TDcJsonReader<CharType>::TDcJsonReader(const CharType* Buf, int Len)
	: TDcJsonReader()	
{
	FDcJsonReaderDetails<CharType>::Reset(this, Buf, Len);
}

template<typename CharType>
void TDcJsonReader<CharType>::AbortAndUninitialize()
{
	State = TDcJsonReader::EState::Uninitialized;
	States.Empty();
	Keys.Empty();

	States.Add(EParseState::Root);
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::FinishRead()
{
	if (State != EState::InProgress)
		return DC_FAIL(DcDJSON, ExpectStateInProgress) << State;

	EDcDataEntry Next;
	DC_TRY(PeekRead(&Next));
	if (Next != EDcDataEntry::Ended)
	{
		return DC_FAIL(DcDJSON, UnexpectedTrailingToken) 
			<< Next
			<< FormatHighlight(Token.Ref);
	}
	else
	{
		check(State == EState::FinishedStr);
		return DcOk();
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::SetNewString(const CharType* InStrPtr, int32 Num)
{
	if (State == EState::InProgress)
		DC_TRY(FinishRead());

	if (State != EState::Uninitialized
		&& State != EState::FinishedStr)
		return DC_FAIL(DcDJSON, ExpectStateUninitializedOrFinished) << State;

	FDcJsonReaderDetails<CharType>::Reset(this, InStrPtr, Num);
	return DcOk();
}

template <typename CharType>
FDcResult TDcJsonReader<CharType>::Coercion(EDcDataEntry ToEntry, bool* OutPtr)
{
	if(bNeedConsumeToken)
	{
		DC_TRY(ConsumeEffectiveToken());
		bNeedConsumeToken = false;
	}

	return ReadOutOk(OutPtr, FDcJsonReaderDetails<CharType>::CheckCoercionRule(this, ToEntry));
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::PeekRead(EDcDataEntry* OutPtr)
{
	if (bNeedConsumeToken)
	{
		DC_TRY(ConsumeEffectiveToken());
		DC_TRY(ReadTokenAsDataEntry(OutPtr));
		bNeedConsumeToken = false;
		return DcOk();
	}
	else
	{
		DC_TRY(ReadTokenAsDataEntry(OutPtr));
		return DcOk();
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadNil()
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::Nil));
	if (Token.Type == ETokenType::Null)
	{
		DC_TRY(CheckNotObjectKey());
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Nil << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadBool(bool* OutPtr)
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::Bool));
	if (Token.Type == ETokenType::True)
	{
		DC_TRY(CheckNotObjectKey());
		ReadOut(OutPtr, true);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else if (Token.Type == ETokenType::False)
	{
		DC_TRY(CheckNotObjectKey());
		ReadOut(OutPtr, false);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Bool << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
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
FDcResult TDcJsonReader<CharType>::CheckObjectDuplicatedKey(const FString& Key)
{
	check(Keys.Num() && IsAtObjectKey());
	if (Keys.Top().Contains(Key))
		return DC_FAIL(DcDJSON, DuplicatedKey) << Key << FormatHighlight(Token.Ref);
	else
		Keys.Top().Add(Key);

	return DcOk();
}

template <typename CharType>
FDcResult TDcJsonReader<CharType>::CheckNotAtEnd()
{
	if (IsAtEnd())
		return DC_FAIL(DcDJSON, UnexpectedEOF) << FormatHighlight(Cur, 1);

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadName(FName* OutPtr)
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::Name));
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		if (IsAtObjectKey())
			DC_TRY(CheckObjectDuplicatedKey(ParsedStr));

		if (ParsedStr.Len() >= NAME_SIZE)
			return DC_FAIL(DcDReadWrite, FNameOverSize);

		if (OutPtr)
			*OutPtr = FName(ParsedStr);

		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Name << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadString(FString* OutPtr)
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::String));
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		if (IsAtObjectKey())
			DC_TRY(CheckObjectDuplicatedKey(ParsedStr));

		ReadOut(OutPtr, MoveTemp(ParsedStr));
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else if (Token.Type == ETokenType::Number)
	{
		ReadOut(OutPtr, Token.Ref.CharsToString());
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::String << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadText(FText* OutPtr)
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::Text));
	if (Token.Type == ETokenType::String)
	{
		FString ParsedStr;
		DC_TRY(ParseStringToken(ParsedStr));

		if (IsAtObjectKey())
			DC_TRY(CheckObjectDuplicatedKey(ParsedStr));

		ReadOut(OutPtr, FText::FromString(MoveTemp(ParsedStr)));
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::Text << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
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
		if (Char == CharType('\0')
			|| SourceUtils::IsLineBreak(Char))
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
			//	explicitly reject error prone ones
			if (Char == CharType('\t'))
				return DC_FAIL(DcDJSON, InvalidControlCharInString) << FormatHighlight(Cur, 1);

			//	then silently load most control chars for now
			if (TIsSame<CharType, ANSICHAR>::Value)
				Token.Flag.bStringHasNonAscii = true;

			Advance();
		}
		else
		{
			if (TIsSame<CharType, ANSICHAR>::Value)
			{
				if (!SourceUtils::IsAscii(Char))
					Token.Flag.bStringHasNonAscii = true;
			}
			
			Advance();
		}
	}

	return DcNoEntry();
}

template <typename CharType>
FString TDcJsonReader<CharType>::ConvertStringTokenToLiteral(SourceRef Ref)
{
	if (TIsSame<CharType, ANSICHAR>::Value
		&& Token.Flag.bStringHasNonAscii)
	{
		//	UTF8 conv when detects non ascii chars
		FUTF8ToTCHAR UTF8Conv((const ANSICHAR*)Ref.GetBeginPtr(), Ref.Num);
		return FString(UTF8Conv.Length(), UTF8Conv.Get());
	}
	else
	{
		return Ref.CharsToString();
	}
}

template <typename CharType>
FName TDcJsonReader<CharType>::ClassId() { return FName(DcJsonReaderDetails::TJsonReaderClassIdSelector<CharType>::Id); }

template <typename CharType>
FName TDcJsonReader<CharType>::GetId() { return ClassId(); }

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ParseStringToken(FString &OutStr)
{
	check(Token.Type == ETokenType::String);

	SourceRef UnquotedRef = Token.Ref;
	UnquotedRef.Begin += 1;
	UnquotedRef.Num -= 2;

	if (!Token.Flag.bStringHasEscapeChar)
	{
		OutStr = ConvertStringTokenToLiteral(UnquotedRef);
		return DcOk();
	}
	else
	{
		FString UnquotedStr = ConvertStringTokenToLiteral(UnquotedRef);
		return FDcJsonReaderDetails<CharType>::ParseQuotedString(this, UnquotedStr, OutStr);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadNumberToken()
{
	Token.Ref.Begin = Cur;
	Token.Flag.Reset();

	CharType Char = PeekChar();
	if (Char == CharType('-'))
	{
		Advance();
		goto READ_NUMBER_MINUS;
	}
	else if (Char == CharType('0'))
	{
		Advance();
		goto READ_NUMBER_ZERO;
	}
	else if (SourceUtils::IsOneToNine(Char))
	{
		Advance();
		goto READ_NUMBER_ANY1;
	}
	else
	{
		DC_TRY(CheckNotAtEnd());
		return DC_FAIL(DcDJSON, NumberInvalidChar) << Char << FormatHighlight(Cur, 1);
	}

READ_NUMBER_MINUS:
	Token.Flag.bNumberIsNegative = true;
	Char = PeekChar();
	if (Char == CharType('0'))
	{
		Advance();
		goto READ_NUMBER_ZERO;
	}
	else if (SourceUtils::IsOneToNine(Char))
	{
		Advance();
		goto READ_NUMBER_ANY1;
	}
	else
	{
		DC_TRY(CheckNotAtEnd());
		return DC_FAIL(DcDJSON, NumberExpectDigitAfterMinus) << Char << FormatHighlight(Cur, 1);
	}

READ_NUMBER_ZERO:
	Char = PeekChar();
	if (Char == CharType('.'))
	{
		Advance();
		goto READ_NUMBER_DECIMAL1;
	}
	else if (Char == CharType('e') || Char == CharType('E'))
	{
		Advance();
		goto READ_NUMBER_EXPONENT;
	}
	else
	{
		goto READ_NUMBER_DONE;
	}
READ_NUMBER_ANY1:
	Char = PeekChar();
	if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_ANY1;
	}
	else if (Char == CharType('.'))
	{
		Advance();
		goto READ_NUMBER_DECIMAL1;
	}
	else if (Char == CharType('e') || Char == CharType('E'))
	{
		Advance();
		goto READ_NUMBER_EXPONENT;
	}
	else
	{
		goto READ_NUMBER_DONE;
	}
READ_NUMBER_DECIMAL1:
	Token.Flag.bNumberHasDecimal = true;
	Token.Flag.NumberDecimalOffset = Cur - Token.Ref.Begin;
	Char = PeekChar();
	if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_DECIMAL2;
	}
	else
	{
		DC_TRY(CheckNotAtEnd());
		return DC_FAIL(DcDJSON, NumberExpectDigitAfterDot) << Char << FormatHighlight(Cur, 1);
	}
READ_NUMBER_DECIMAL2:
	Char = PeekChar();
	if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_DECIMAL2;
	}
	else if (Char == CharType('e') || Char == CharType('E'))
	{
		Advance();
		goto READ_NUMBER_EXPONENT;
	}
	else
	{
		goto READ_NUMBER_DONE;
	}
READ_NUMBER_EXPONENT:
	Token.Flag.bNumberHasExp = true;
	Char = PeekChar();
	if (Char == CharType('-') || Char == CharType('+'))
	{
		Advance();
		goto READ_NUMBER_SIGN;
	}
	else if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_ANY2;
	}
	else
	{
		DC_TRY(CheckNotAtEnd());
		return DC_FAIL(DcDJSON, NumberExpectSignDigitAfterExp) << Char << FormatHighlight(Cur, 1);
	}

READ_NUMBER_SIGN:
	Char = PeekChar();
	if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_ANY2;
	}
	else
	{
		DC_TRY(CheckNotAtEnd());
		return DC_FAIL(DcDJSON, NumberExpectDigitAfterExpSign) << Char << FormatHighlight(Cur, 1);
	}
READ_NUMBER_ANY2:
	Char = PeekChar();
	if (SourceUtils::IsDigit(Char))
	{
		Advance();
		goto READ_NUMBER_ANY2;
	}
	else
	{
		goto READ_NUMBER_DONE;
	}

READ_NUMBER_DONE:

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
FDcResult TDcJsonReader<CharType>::ReadTokenAsDataEntry(EDcDataEntry* OutPtr)
{
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
			ReadOut(OutPtr, FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type));
			return DcOk();
		}
	default:
		return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::CheckConsumeToken(EDcDataEntry Expect)
{
	if (bNeedConsumeToken)
	{
		EDcDataEntry Actual;
		DC_TRY(ConsumeEffectiveToken());
		DC_TRY(ReadTokenAsDataEntry(&Actual));
		if (Actual != Expect
			&& !FDcJsonReaderDetails<CharType>::CheckCoercionRule(this, Expect))
			return DC_FAIL(DcDReadWrite, DataTypeMismatchNoCoercion)
				<< Expect << Actual
				<< FormatHighlight(Token.Ref);
	}

	//	setting need consume token for the next one
	bNeedConsumeToken = true;
	return DcOk();
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
	else if (TopState == EParseState::Root)
	{
		return DcOk();
	}
	else
	{
		return DcNoEntry();
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadMapRoot()
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::MapRoot));
	if (Token.Type == ETokenType::CurlyOpen)
	{
		DC_TRY(CheckNotObjectKey());
		PushTopState(EParseState::Object);
		bTopObjectAtValue = false;
		Keys.AddDefaulted();
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::MapRoot << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadMapEnd()
{
	if (GetTopState() != EParseState::Object)
		return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
	
	DC_TRY(CheckConsumeToken(EDcDataEntry::MapEnd));
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
			<< EDcDataEntry::MapEnd << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadArrayRoot()
{
	DC_TRY(CheckConsumeToken(EDcDataEntry::ArrayRoot));
	if (Token.Type == ETokenType::SquareOpen)
	{
		DC_TRY(CheckNotObjectKey());
		PushTopState(EParseState::Array);
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::ArrayRoot << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadArrayEnd()
{
	if (GetTopState() != EParseState::Array)
		return DC_FAIL(DcDJSON, UnexpectedToken) << FormatHighlight(Token.Ref);
	
	DC_TRY(CheckConsumeToken(EDcDataEntry::ArrayEnd));
	if (Token.Type == ETokenType::SquareClose)
	{
		PopTopState(EParseState::Array);
		DC_TRY(EndTopRead());
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDJSON, ReadTypeMismatch)
			<< EDcDataEntry::ArrayEnd << FDcJsonReaderDetails<CharType>::TokenTypeToDataEntry(Token.Type)
			<< FormatHighlight(Token.Ref);
	}
}

template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt8(int8* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadSignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt16(int16* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadSignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt32(int32* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadSignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadInt64(int64* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadSignedInteger(this, OutPtr); }

template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt8(uint8* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadUnsignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt16(uint16* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadUnsignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt32(uint32* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadUnsignedInteger(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadUInt64(uint64* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadUnsignedInteger(this, OutPtr); }

template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadFloat(float* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadFloating(this, OutPtr); }
template<typename CharType> FDcResult TDcJsonReader<CharType>::ReadDouble(double* OutPtr) { return FDcJsonReaderDetails<CharType>::ReadFloating(this, OutPtr); }

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ConsumeRawToken()
{
	check(State == EState::InProgress);
	if (CachedNext.IsValid())
	{
		Token = CachedNext;
		CachedNext.Reset();
		return DcOk();
	}

	if (IsAtEnd())
	{
		if (GetTopState() == EParseState::Object)
		{
			return DC_FAIL(DcDJSON, EndUnclosedObject) << FormatHighlight(Cur, 1);
		}
		else if (GetTopState() == EParseState::Array)
		{
			return DC_FAIL(DcDJSON, EndUnclosedArray) << FormatHighlight(Cur, 1);
		}
		else
		{
			Token.Type = ETokenType::EOF_;
			//	keep the token for diagnostic highlight
			State = EState::FinishedStr;
			return DcOk();
		}
	}

	auto _ConsumeSingleCharToken = [this](ETokenType TokenType) {
		Token.Type = TokenType;
		Token.Ref.Begin = Cur;
		Token.Ref.Num = 1;
		Advance();
		return DcOk();
	};

	CharType Char = PeekChar();
	switch (Char)
	{
		case CharType('/'):
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
			break;
		}
		case CharType('{'):
			return _ConsumeSingleCharToken(ETokenType::CurlyOpen);
		case CharType('}'):
			return _ConsumeSingleCharToken(ETokenType::CurlyClose);
		case CharType('['):
			return _ConsumeSingleCharToken(ETokenType::SquareOpen);
		case CharType(']'):
			return _ConsumeSingleCharToken(ETokenType::SquareClose);
		case CharType(':'):
			return _ConsumeSingleCharToken(ETokenType::Colon);
		case CharType(','):
			return _ConsumeSingleCharToken(ETokenType::Comma);
		case CharType('t'):
			{
				const static CharType _TRUE[] = { 't','r','u','e',0 };

				DC_TRY(ReadWordExpect(_TRUE));
				Token.Type = ETokenType::True;
				return DcOk();
			}
		case CharType('f'):
			{
				const static CharType _FALSE[] = { 'f','a','l','s','e',0 };

				DC_TRY(ReadWordExpect(_FALSE));
				Token.Type = ETokenType::False;
				return DcOk();
			}
		case CharType('n'):
			{
				const static CharType _NULL[] = { 'n','u','l','l',0 };

				DC_TRY(ReadWordExpect(_NULL));
				Token.Type = ETokenType::Null;
				return DcOk();
			}
		case CharType('"'):
				return ReadStringToken();
		default:
			{
				if (SourceUtils::IsWhitespace(Char))
				{
					ReadWhiteSpace();
					return DcOk();
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

	return DcNoEntry();
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
	check(State != EState::Uninitialized && State != EState::Invalid);
	check(Cur >= 0);
	return Cur + N >= Buf.Num;
}

template<typename CharType>
void TDcJsonReader<CharType>::Advance()
{
	AdvanceN(1);
}

template<typename CharType>
void TDcJsonReader<CharType>::AdvanceN(int N)
{
	check(N > 0);
	check(!IsAtEnd(N - 1));
	Cur += N;
	Loc.Column += N;
}

template<typename CharType>
CharType TDcJsonReader<CharType>::PeekChar(int N)
{
	if (IsAtEnd(N))
		return CharType('\0');
	else
		return Buf.Buffer[Cur + N];
}

template<typename CharType>
FDcResult TDcJsonReader<CharType>::ReadWordExpect(const CharType* Word)
{
	int32 WordLen = CString::Strlen(Word);
	SourceRef WordRef = Token.Ref;
	WordRef.Begin = Cur;
	WordRef.Num = WordLen;

	for (int Ix = 0; Ix < WordLen; Ix++)
	{
		if (CharType(Word[Ix]) != PeekChar(Ix))
		{
			return DC_FAIL(DcDJSON, ExpectWordButNotFound)
				<< Word << WordRef.CharsToString()
				<< FormatHighlight(WordRef);
		}
	}

	Token.Ref = WordRef;
	AdvanceN(WordLen);
	return DcOk();
}

template<typename CharType>
FDcDiagnosticHighlight TDcJsonReader<CharType>::FormatHighlight(SourceRef SpanRef)
{
	FDcDiagnosticHighlight OutHighlight(this, ClassId().ToString());
	OutHighlight.FileContext.Emplace();
	OutHighlight.FileContext->Loc = Loc;
	OutHighlight.FileContext->FilePath = DiagFilePath.IsEmpty() ? TEXT("<in-memory>") : DiagFilePath;

	THightlightFormatter<CharType> Highlighter;
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, Loc.Line);

	if (OutHighlight.Formatted.IsEmpty())
		OutHighlight.Formatted = TEXT("<contents empty>");
	
	return OutHighlight;
}

template<typename CharType>
void TDcJsonReader<CharType>::FormatDiagnostic(FDcDiagnostic& Diag)
{
	Diag << FormatHighlight(Token.Ref);
}


template struct DATACONFIGCORE_API TDcJsonReader<ANSICHAR>;
template struct DATACONFIGCORE_API TDcJsonReader<WIDECHAR>;

