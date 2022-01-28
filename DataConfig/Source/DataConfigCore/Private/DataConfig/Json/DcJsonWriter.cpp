#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Source/DcHighlightFormatter.h"
#include "DataConfig/Misc/DcTemplateUtils.h"


//	compile hack for c++14 constexpr
template<typename CharType> constexpr CharType TDcJsonWriter<CharType>::_DEFAULT_SPACING_LITERAL[];
template<typename CharType> constexpr CharType TDcJsonWriter<CharType>::_DEFAULT_INDENT_LITERAL[];
template<typename CharType> constexpr CharType TDcJsonWriter<CharType>::_DEFAULT_NEWLINE_LITERAL[];
template<typename CharType> constexpr CharType TDcJsonWriter<CharType>::_EMPTY_LITERAL[];
template<typename CharType> constexpr typename TDcJsonWriter<CharType>::ConfigType TDcJsonWriter<CharType>::DefaultConfig;
template<typename CharType> constexpr typename TDcJsonWriter<CharType>::ConfigType TDcJsonWriter<CharType>::CondenseConfig;

namespace DcJsonWriterDetails
{

FORCEINLINE bool IsValidWriteScalar(EDcDataEntry Entry)
{
	switch (Entry)
	{
		case EDcDataEntry::Nil:
		case EDcDataEntry::Bool:
		case EDcDataEntry::Name:
		case EDcDataEntry::String:
		case EDcDataEntry::Text:
		case EDcDataEntry::Float:
		case EDcDataEntry::Double:
		case EDcDataEntry::Int8:
		case EDcDataEntry::Int16:
		case EDcDataEntry::Int32:
		case EDcDataEntry::Int64:
		case EDcDataEntry::UInt8:
		case EDcDataEntry::UInt16:
		case EDcDataEntry::UInt32:
		case EDcDataEntry::UInt64:
			return true;
		default:
			return false;
	}
}

FORCEINLINE bool IsValidWriterKey(EDcDataEntry Entry)
{
	switch (Entry)
	{
		case EDcDataEntry::Name:
		case EDcDataEntry::String:
		case EDcDataEntry::Text:
			return true;
		default:
			return false;
	}
}

static FORCEINLINE void WriteSbDispatch(TDcJsonWriter<WIDECHAR>::StringBuilder& Sb, const FString& Value)
{
	Sb << Value;
}

static FORCEINLINE void WriteSbDispatch(TDcJsonWriter<ANSICHAR>::StringBuilder& Sb, const FString& Value)
{
	Sb << FTCHARToUTF8(*Value).Get();
}

template<typename CharType>
struct FNumericDispatch
{
	using StringBuilder = typename TDcJsonWriter<CharType>::StringBuilder;

	static FORCEINLINE void _WriteInt64(StringBuilder& Sb, const int64& Value)
	{
		const static CharType _INT64_FMT[] = { '%', 'l', 'l', 'd', 0 };
		Sb.Appendf(_INT64_FMT, Value);
	}

	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const int8& Value) { _WriteInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const int16& Value) { _WriteInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const int32& Value) { _WriteInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const int64& Value) { _WriteInt64(Sb, Value); }

	static FORCEINLINE void _WriteUInt64(StringBuilder& Sb, const int64& Value)
	{
		const static CharType _UINT64_FMT[] = { '%', 'l', 'l', 'u', 0 };
		Sb.Appendf(_UINT64_FMT, Value);
	}

	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const uint8& Value) { _WriteUInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const uint16& Value) { _WriteUInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const uint32& Value) { _WriteUInt64(Sb, Value); }
	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const uint64& Value) { _WriteUInt64(Sb, Value); }

	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const float& Value)
	{
		const static CharType _FLOAT_FMT[] = { '%', 'g', 0 };
		Sb.Appendf(_FLOAT_FMT, Value);
	}

	static FORCEINLINE void WriteNumericDispatch(StringBuilder& Sb, const double& Value)
	{
		const static CharType _DOUBLE_FMT[] = { '%', '.', '1', '7', 'g', 0 };
		Sb.Appendf(_DOUBLE_FMT, Value);
	}
};

template<typename CharType>
struct TJsonWriterClassIdSelector;
template<> struct TJsonWriterClassIdSelector<ANSICHAR> { static constexpr const TCHAR* Id = TEXT("AnsiCharDcJsonWriter"); };
template<> struct TJsonWriterClassIdSelector<WIDECHAR> { static constexpr const TCHAR* Id = TEXT("WideCharDcJsonWriter"); };

} // namespace DcJsonWriterDetails

template<typename CharType>
struct FDcJsonWriterDetails
{

using TSelf = TDcJsonWriter<CharType>;

static FORCEINLINE void WriteLineEnd(TSelf* Self) { Self->Sb << Self->Config.LineEndLiteral; }

static FORCEINLINE void WriteRightSpacing(TSelf* Self) { Self->Sb << Self->Config.RightSpacingLiteral; }

static FORCEINLINE void WriteIndentation(TSelf* Self)
{
	for (int Ix = 0; Ix < Self->State.Indent; Ix++)
		Self->Sb << Self->Config.IndentLiteral;
}

static FORCEINLINE void ConsumeWriteComma(TSelf* Self)
{
	if (Self->State.bNeedComma)
	{
		Self->Sb << CharType(',');

		WriteLineEnd(Self);
		WriteIndentation(Self);

		Self->State.bNeedComma = false;
	}
}

static FORCEINLINE void ConsumeWriteRightSpacing(TSelf* Self)
{
	if (Self->State.bNeedRightSpacing)
	{
		WriteRightSpacing(Self);
		Self->State.bNeedRightSpacing = false;
	}
}

static FORCEINLINE void ConsumeWriteNestedRightSpacing(TSelf* Self, bool bNewLine)
{
	if (Self->State.bNeedRightSpacing)
	{
		if (bNewLine)
		{
			WriteLineEnd(Self);
			WriteIndentation(Self);
		}
		else
		{
			WriteRightSpacing(Self);
		}

		Self->State.bNeedRightSpacing = false;
	}
}

static FORCEINLINE void ConsumeNeedNewLineAndIndent(TSelf* Self)
{
	if (Self->State.bNeedNewlineAndIndent)
	{
		WriteLineEnd(Self);
		WriteIndentation(Self);
		Self->State.bNeedNewlineAndIndent = false;
	}
}


static void WriteEscapedString(TSelf* Self, const FString& Str)
{
	//  note how we always read from TCHAR but write by CharType
	using StringSourceUtils = TDcCSourceUtils<TCHAR>;

	//	pre pass to scan for escape chars
	bool bNeedEscape = false;
	for (int Ix = 0; Ix < Str.Len(); Ix++)
	{
		TCHAR Ch = Str[Ix];

		switch (Ch)
		{
			case TCHAR('\\'): 
			case TCHAR('\n'):
			case TCHAR('\t'):
			case TCHAR('\b'):
			case TCHAR('\f'):
			case TCHAR('\r'):
			case TCHAR('\"'):
				bNeedEscape = true;
				break;
			default:
				break;
		}

		if (StringSourceUtils::IsControl(Ch))
			bNeedEscape = true;

		if (bNeedEscape)
			break;
	}

	if (!bNeedEscape)
	{
		Self->Sb << CharType('"');
		DcJsonWriterDetails::WriteSbDispatch(Self->Sb, Str);
		Self->Sb << CharType('"');
		return;
	}

	//	proper escaping
	Self->Sb << CharType('"');
	for (int Ix = 0; Ix < Str.Len(); Ix++)
	{
		TCHAR Ch = Str[Ix];

		const static CharType _SLASH_ESCAPED[] = { '\\', '\\', 0 };
		const static CharType _N_ESCAPED[] = { '\\', 'n', 0 };
		const static CharType _T_ESCAPED[] = { '\\', 't', 0 };
		const static CharType _B_ESCAPED[] = { '\\', 'b', 0 };
		const static CharType _F_ESCAPED[] = { '\\', 'f', 0 };
		const static CharType _R_ESCAPED[] = { '\\', 'r', 0 };
		const static CharType _DQUOTE_ESCAPED[] = { '\\', '"', 0 };

		switch (Ch)
		{
			case TCHAR('\\'): Self->Sb << _SLASH_ESCAPED; break;
			case TCHAR('\n'): Self->Sb << _N_ESCAPED; break;
			case TCHAR('\t'): Self->Sb << _T_ESCAPED; break;
			case TCHAR('\b'): Self->Sb << _B_ESCAPED; break;
			case TCHAR('\f'): Self->Sb << _F_ESCAPED; break;
			case TCHAR('\r'): Self->Sb << _R_ESCAPED; break;
			case TCHAR('\"'): Self->Sb << _DQUOTE_ESCAPED; break;
			default:
			{
				if (StringSourceUtils::IsControl(Ch))
				{
					const static CharType _CONTROL_ESCAPE_FMT[] = { '\\', 'u', '%', '0', '4', 'x', 0 };
					Self->Sb.Appendf(_CONTROL_ESCAPE_FMT, Ch);
				}
				else
				{
					Self->Sb.Append(Ch);
				}
				break;
			}
		}
	}
	Self->Sb << CharType('"');
}


static FORCEINLINE FDcResult CheckAtValuePosition(TSelf* Self)
{
	using EWriteState = typename TSelf::EWriteState;

	EWriteState WriteState = Self->GetTopState();
	if (WriteState == EWriteState::Object
		&& !Self->State.bTopObjectAtValue)
	{
		return DC_FAIL(DcDJSON, ExpectStringAtObjectKey);
	}
	else
	{
		return DcOk();
	}
}

static FORCEINLINE void BeginWriteValuePosition(TSelf* Self)
{
	ConsumeNeedNewLineAndIndent(Self);
	ConsumeWriteComma(Self);
	ConsumeWriteRightSpacing(Self);
}

static FORCEINLINE void EndWriteValuePosition(TSelf* Self)
{
	Self->State.bNeedComma = true;
	Self->State.bTopObjectAtValue = false;
	Self->State.bTopContainerNotEmpty = true;
}

static void WriteString(TSelf* Self, const FString& Value)
{
	using EWriteState = typename TSelf::EWriteState;

	EWriteState WriteState = Self->GetTopState();
	if (WriteState == TSelf::EWriteState::Object
		&& !Self->State.bTopObjectAtValue)
	{
		//	at key pos
		ConsumeNeedNewLineAndIndent(Self);
		ConsumeWriteComma(Self);

		WriteEscapedString(Self, Value);
		Self->Sb << Self->Config.LeftSpacingLiteral; 
		Self->Sb << ":";

		Self->State.bNeedRightSpacing = true;
		Self->State.bTopObjectAtValue = true;
	}
	else
	{
		BeginWriteValuePosition(Self);
		WriteEscapedString(Self, Value);
		EndWriteValuePosition(Self);
	}
}

template<typename TInt>
static FDcResult WriteNumeric(TSelf* Self, const TInt& Value)
{
	DC_TRY(CheckAtValuePosition(Self));

	BeginWriteValuePosition(Self);
	DcJsonWriterDetails::FNumericDispatch<CharType>::WriteNumericDispatch(Self->Sb, Value);
	EndWriteValuePosition(Self);
	return DcOk();
}

};

template<typename CharType>
TDcJsonWriter<CharType>::TDcJsonWriter()
	: TDcJsonWriter(DefaultConfig)
{}

template<typename CharType>
TDcJsonWriter<CharType>::TDcJsonWriter(ConfigType InConfig)
	: Config(InConfig)
{
	States.Add(EWriteState::Root);
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteNil()
{
	using Details = FDcJsonWriterDetails<CharType>;

	DC_TRY(Details::CheckAtValuePosition(this));

	Details::BeginWriteValuePosition(this);
	const static CharType _NULL[] = { 'n','u','l','l',0 };
	Sb << _NULL;
	Details::EndWriteValuePosition(this);

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteBool(bool bValue)
{
	using Details = FDcJsonWriterDetails<CharType>;

	DC_TRY(Details::CheckAtValuePosition(this));

	Details::BeginWriteValuePosition(this);

	const static CharType _TRUE[] = { 't','r','u','e',0 };
	const static CharType _FALSE[] = { 'f','a','l','s','e',0 };

	if (bValue)
		Sb << _TRUE;
	else
		Sb << _FALSE;
	Details::EndWriteValuePosition(this);

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteString(const FString& Value)
{
	FDcJsonWriterDetails<CharType>::WriteString(this, Value);
	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteText(const FText& Value)
{
	FDcJsonWriterDetails<CharType>::WriteString(this, Value.ToString());
	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteName(const FName& Value)
{
	FDcJsonWriterDetails<CharType>::WriteString(this, Value.ToString());
	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteMapRoot()
{
	using Details = FDcJsonWriterDetails<CharType>;

	DC_TRY(Details::CheckAtValuePosition(this));

	Details::ConsumeNeedNewLineAndIndent(this);
	Details::ConsumeWriteComma(this);
	Details::ConsumeWriteNestedRightSpacing(this, Config.bNestedObjectStartsOnNewLine);

	States.Push(EWriteState::Object);
	Sb << CharType('{');
	++State.Indent;
	State.bTopContainerNotEmpty = false;
	State.bNeedNewlineAndIndent = true;
	State.bTopObjectAtValue = false;

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteMapEnd()
{
	using Details = FDcJsonWriterDetails<CharType>;

	EWriteState Popped = States.Pop();
	if (Popped != EWriteState::Object
		|| State.bTopObjectAtValue)
		return DC_FAIL(DcDJSON, UnexpectedObjectEnd);

	--State.Indent;
	if (State.bTopContainerNotEmpty)
	{
		Details::WriteLineEnd(this);
		Details::WriteIndentation(this);
	}
	State.bNeedNewlineAndIndent = false;

	Sb << CharType('}');

	Details::EndWriteValuePosition(this);
	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteArrayRoot()
{
	using Details = FDcJsonWriterDetails<CharType>;

	DC_TRY(Details::CheckAtValuePosition(this));

	Details::ConsumeNeedNewLineAndIndent(this);
	Details::ConsumeWriteComma(this);
	Details::ConsumeWriteNestedRightSpacing(this, Config.bNestedArrayStartsOnNewLine);

	States.Push(EWriteState::Array);
	Sb << CharType('[');
	++State.Indent;
	State.bTopContainerNotEmpty = false;
	State.bNeedNewlineAndIndent = true;

	return DcOk();
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteArrayEnd()
{
	using Details = FDcJsonWriterDetails<CharType>;

	EWriteState Popped = States.Pop();
	if (Popped != EWriteState::Array)
		return DC_FAIL(DcDJSON, UnexpectedArrayEnd);

	State.Indent--;
	if (State.bTopContainerNotEmpty)
	{
		Details::WriteLineEnd(this);
		Details::WriteIndentation(this);
	}
	State.bNeedNewlineAndIndent = false;

	Sb << CharType(']');

	Details::EndWriteValuePosition(this);
	return DcOk();
}

template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteInt8(const int8& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteInt16(const int16& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteInt32(const int32& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteInt64(const int64& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }

template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteUInt8(const uint8& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteUInt16(const uint16& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteUInt32(const uint32& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteUInt64(const uint64& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }

template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteFloat(const float& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }
template<typename CharType> FDcResult TDcJsonWriter<CharType>::WriteDouble(const double& Value) { return FDcJsonWriterDetails<CharType>::template WriteNumeric(this, Value); }

template <typename CharType>
void TDcJsonWriter<CharType>::FormatDiagnostic(FDcDiagnostic& Diag)
{
	FDcDiagnosticHighlight OutHighlight(this, ClassId().ToString());

	THightlightFormatter<CharType> Highlighter;

	TDcSourceView<CharType> View{Sb.GetData(), Sb.Len()};
	TDcSourceRef<CharType> SpanRef{&View, Sb.Len() - 1, 1};
	FString FormatHeader(TEXT("Json Last Writes:"));
	OutHighlight.Formatted = Highlighter.FormatHighlight(SpanRef, INDEX_NONE, &FormatHeader);

	if (OutHighlight.Formatted.IsEmpty())
		OutHighlight.Formatted = TEXT("<contents empty>");

	Diag << MoveTemp(OutHighlight);
}

template <typename CharType>
FName TDcJsonWriter<CharType>::ClassId() { return FName(DcJsonWriterDetails::TJsonWriterClassIdSelector<CharType>::Id); }
template <typename CharType>
FName TDcJsonWriter<CharType>::GetId() { return ClassId(); }

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::WriteRawStringValue(const FString& Value)
{
	using Details = FDcJsonWriterDetails<CharType>;

	DC_TRY(Details::CheckAtValuePosition(this));

	Details::BeginWriteValuePosition(this);
	DcJsonWriterDetails::WriteSbDispatch(Sb, Value);
	Details::EndWriteValuePosition(this);
	return DcOk();
}

template <typename CharType>
void TDcJsonWriter<CharType>::CancelWriteComma()
{
	State.bNeedComma = false;
}

template<typename CharType>
FDcResult TDcJsonWriter<CharType>::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	using namespace  DcJsonWriterDetails;

	EWriteState WriteState = GetTopState();
	if (WriteState == EWriteState::Array)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::ArrayEnd
			|| Next == EDcDataEntry::MapRoot
			|| Next == EDcDataEntry::ArrayRoot
			|| IsValidWriteScalar(Next)
		);
	}
	else if (WriteState == EWriteState::Object)
	{
		if (State.bTopObjectAtValue)
		{
			return ReadOutOk(bOutOk, IsValidWriteScalar(Next)
				|| Next == EDcDataEntry::MapRoot
				|| Next == EDcDataEntry::ArrayRoot
			);
		}
		else
		{
			return ReadOutOk(bOutOk, Next == EDcDataEntry::MapEnd
				|| IsValidWriterKey(Next)
			);
		}
	}
	else if (WriteState == EWriteState::Root)
	{
		return ReadOutOk(bOutOk, Next == EDcDataEntry::Ended
			|| Next == EDcDataEntry::MapRoot
			|| Next == EDcDataEntry::ArrayRoot
			|| IsValidWriteScalar(Next));
	}
	else
	{
		return DcNoEntry();
	}
}

template struct DATACONFIGCORE_API TDcJsonWriter<ANSICHAR>;
template struct DATACONFIGCORE_API TDcJsonWriter<WIDECHAR>;


