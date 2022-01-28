#pragma once

#include "CoreMinimal.h"
#include "Misc/StringBuilder.h"
#include "DataConfig/Source/DcSourceUtils.h"
#include "DataConfig/Writer/DcWriter.h"

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25

namespace DcJsonWriterDetails
{
template <typename CharType> struct TSBTypeSelector;
template <> struct TSBTypeSelector<ANSICHAR> { using Type = TAnsiStringBuilder<1024>; };
template <> struct TSBTypeSelector<WIDECHAR> { using Type = TStringBuilder<1024>; };
} // namespace DcJsonWriterDetails

#endif

template<typename CharType>
struct TDcJsonWriter : public FDcWriter, private FNoncopyable
{
	using TSelf = TDcJsonWriter;

#if ENGINE_MAJOR_VERSION == 5
	using StringBuilder = TStringBuilderWithBuffer<CharType, 1024>;
#else
	#if ENGINE_MINOR_VERSION >= 26
	using StringBuilder = TStringBuilderWithBuffer<CharType, 1024>;
	#else
	using StringBuilder = typename DcJsonWriterDetails::TSBTypeSelector<CharType>::Type;
	#endif
#endif

	using SourceUtils = TDcCSourceUtils<CharType>;
	using CString = TCString<CharType>;


	struct ConfigType
	{
		const CharType* LeftSpacingLiteral;		//	Left side spacing around `:`
		const CharType* RightSpacingLiteral;	//	Right side spacing around `:`
		const CharType* IndentLiteral;		
		const CharType* LineEndLiteral;

		bool bNestedArrayStartsOnNewLine;		//	aka C Braces Style on nested array
		bool bNestedObjectStartsOnNewLine;		//	aka C Braces Style on nested map
	};

	constexpr static CharType _DEFAULT_SPACING_LITERAL[] = { ' ', 0 };
	constexpr static CharType _DEFAULT_INDENT_LITERAL[] = { ' ',' ',' ',' ',0 };
	constexpr static CharType _DEFAULT_NEWLINE_LITERAL[] = { '\n', 0 };

	constexpr static ConfigType DefaultConfig = {
		_DEFAULT_SPACING_LITERAL,
		_DEFAULT_SPACING_LITERAL,
		_DEFAULT_INDENT_LITERAL,
		_DEFAULT_NEWLINE_LITERAL,
		false,
		false
	};

	constexpr static CharType _EMPTY_LITERAL[] = { 0 };
	constexpr static ConfigType CondenseConfig = {
		_EMPTY_LITERAL,
		_EMPTY_LITERAL,
		_EMPTY_LITERAL,
		_EMPTY_LITERAL,
		false,
		false
	};


	StringBuilder Sb;
	ConfigType Config;

	enum class EWriteState : uint8
	{
		Root,
		Object,
		Array,
	};

	TArray<EWriteState, TInlineAllocator<8>> States;
	using FKeys = TArray<FName, TInlineAllocator<8>>;
	TArray<FKeys, TInlineAllocator<8>> Keys;

	FORCEINLINE EWriteState GetTopState() { return States.Top(); }

	struct FState
	{
		uint8 bTopObjectAtValue : 1;
		uint8 bNeedComma : 1;
		uint8 bNeedRightSpacing : 1;
		uint8 bTopContainerNotEmpty :1;
		uint8 bNeedNewlineAndIndent :1;

		uint8 Indent;

		FORCEINLINE void Reset() { *this = FState{}; }
	};

	FState State = {};

	TDcJsonWriter();
	TDcJsonWriter(ConfigType InConfig);

	FDcResult PeekWrite(EDcDataEntry Next, bool* bOutOk) override;

	FDcResult WriteNil() override;
	FDcResult WriteBool(bool Value) override;
	FDcResult WriteString(const FString& Value) override;

	FDcResult WriteText(const FText& Value) override;
	FDcResult WriteName(const FName& Value) override;

	FDcResult WriteMapRoot() override;
	FDcResult WriteMapEnd() override;
	FDcResult WriteArrayRoot() override;
	FDcResult WriteArrayEnd() override;

	FDcResult WriteInt8(const int8& Value) override;
	FDcResult WriteInt16(const int16& Value) override;
	FDcResult WriteInt32(const int32& Value) override;
	FDcResult WriteInt64(const int64& Value) override;

	FDcResult WriteUInt8(const uint8& Value) override;
	FDcResult WriteUInt16(const uint16& Value) override;
	FDcResult WriteUInt32(const uint32& Value) override;
	FDcResult WriteUInt64(const uint64& Value) override;

	FDcResult WriteFloat(const float& Value) override;
	FDcResult WriteDouble(const double& Value) override;

	void FormatDiagnostic(FDcDiagnostic& Diag) override;

	static FName ClassId(); 
	FName GetId() override;

	///	Unsafe extension to write arbitrary string at value position
	FDcResult WriteRawStringValue(const FString& Value);
	///	Cancel write comma for ndjson like spacing
	void CancelWriteComma();
};

template<typename CharType>
struct TDcPrettyJsonWriter : public TDcJsonWriter<CharType>
{
	using Super = TDcJsonWriter<CharType>;

	TDcPrettyJsonWriter<CharType>() : Super() {}
};

template<typename CharType>
struct TDcCondensedJsonWriter : public TDcJsonWriter<CharType>
{
	using Super = TDcJsonWriter<CharType>;

	TDcCondensedJsonWriter<CharType>() : Super(Super::CondenseConfig) {}
};

using FDcJsonWriter = TDcJsonWriter<TCHAR>;
using FDcPrettyJsonWriter = TDcPrettyJsonWriter<TCHAR>;
using FDcCondensedJsonWriter = TDcCondensedJsonWriter<TCHAR>;

using FDcWideJsonWriter = TDcJsonWriter<WIDECHAR>;
using FDcWidePrettyJsonWriter = TDcPrettyJsonWriter<WIDECHAR>;
using FDcWideCondensedJsonWriter = TDcCondensedJsonWriter<WIDECHAR>;

using FDcAnsiJsonWriter = TDcJsonWriter<ANSICHAR>;
using FDcAnsiPrettyJsonWriter = TDcPrettyJsonWriter<ANSICHAR>;
using FDcAnsiCondensedJsonWriter = TDcCondensedJsonWriter<ANSICHAR>;

