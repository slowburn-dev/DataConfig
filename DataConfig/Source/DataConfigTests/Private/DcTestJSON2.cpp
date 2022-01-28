#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "Misc/FileHelper.h"

namespace DcTestJsonDetails
{

static FDcResult RoundtripJsonLiteral(FAutomationTestBase* Self, const FString& Str, FDcJsonWriter&& Writer)
{
	FString NormalizedStr = DcReindentStringLiteral(Str);

	FDcJsonReader Reader(Str);
	FDcPipeVisitor PipeVisitor(&Reader, &Writer);

	DC_TRY(PipeVisitor.PipeVisit());

	//	amend trailing line end
	Writer.Sb.Append('\n');

	FString Result = Writer.Sb.ToString();

	if (!Self->TestEqual(TEXT("Json Writer Roundtrip"), Result, NormalizedStr))
	{
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("Json Writer Roundtrip Failed"));
	}

	return DcOk();
}

} // namespace DcTestJsonDetails

DC_TEST("DataConfig.Core.JSON.ReadStrings")
{
	{
		auto _ReadJsonStrEqual = [](FDcAutomationBase* Self, FString InStr, FString ExpectStr) -> FDcResult
		{
			FDcJsonReader Reader(InStr);
			FString ParsedStr;
			DC_TRY(Reader.ReadString(&ParsedStr));
			if (!Self->TestEqual("ReadJson String Literal", ParsedStr, ExpectStr))
				return DC_FAIL(DcDCommon, CustomMessage)
					<< FString::Printf(TEXT("ReadJson String Literal"));
			
			return DcOk();
		};

		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "" )"), TEXT("")));
		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "Hello\nWorld" )"), TEXT("Hello\nWorld")));
		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "\"\\/\b\f\n\r\t" )"), TEXT("\"\\/\b\f\n\r\t")));
		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "\u0024" )"), TEXT("$")));
		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "\u00A2" )"), TEXT("\u00A2")));
		UTEST_OK("ReadJson String Literal", _ReadJsonStrEqual(this, TEXT(R"( "\uD834\uDD1E" )"), TEXT("\xD834\xDD1E")));
	}

	{
		auto _ReadJsonStr = [](FDcAutomationBase* Self, FString InStr) -> FDcResult
		{
			FDcJsonReader Reader(InStr);
			FString ParsedStr;
			return Reader.ReadString(&ParsedStr);
		};

		UTEST_DIAG("ReadJson Expect Fail", _ReadJsonStr(this, TEXT(R"( "\O" )")), DcDJSON, InvalidStringEscaping);
		UTEST_DIAG("ReadJson Expect Fail", _ReadJsonStr(this, TEXT(R"( "\u123" )")), DcDJSON, InvalidStringEscaping);
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.WriterErrors")
{
	{
		FDcJsonWriter Writer;

		UTEST_OK("WriterJson Errors", Writer.WriteMapRoot());
		UTEST_DIAG("WriterJson Errors", Writer.WriteBool(true), DcDJSON, ExpectStringAtObjectKey);
	}

	{
		FDcJsonWriter Writer;

		UTEST_OK("WriterJson Errors", Writer.WriteMapRoot());
		UTEST_DIAG("WriterJson Errors", Writer.WriteNil(), DcDJSON, ExpectStringAtObjectKey);
	}

	{
		FDcJsonWriter Writer;

		UTEST_DIAG("WriterJson Errors", Writer.WriteMapEnd(), DcDJSON, UnexpectedObjectEnd);
	}


	{
		FDcJsonWriter Writer;

		UTEST_DIAG("WriterJson Errors", Writer.WriteArrayEnd(), DcDJSON, UnexpectedArrayEnd);
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.Writer1")
{
	using namespace DcTestJsonDetails;

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{
			"Hello" : "JSON",
			"Truthy" : true,
			"Inty" : 123,
			"Falsy" : false,
			"Nully" : null,
			"Arrayy" : [
				"JSON",
				true,
				false,
				123,
				null
			]
		} 

	)"), FDcPrettyJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{
			"Foo" : "Bar",
			"Nest1" : {
				"Inner1" : "Foo",
				"Inner2" : "Bar"
			},
			"Emtpy1" : [],
			"Emtpy2" : {},
			"Outer" : "Dart"
		} 

	)"), FDcPrettyJsonWriter()));

	{
		FDcPrettyJsonWriter NestOnNewLineWriter;
		NestOnNewLineWriter.Config.bNestedArrayStartsOnNewLine = true;
		NestOnNewLineWriter.Config.bNestedObjectStartsOnNewLine = true;

		UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

			{
				"Foo" : "Bar",
				"Nest1" : 
				{
					"Inner1" : "Foo",
					"Inner2" : "Bar"
				},
				"Emtpy1" :
				[],
				"Emtpy2" :
				{},
				"Outer" : "Dart"
			} 

		)"), MoveTemp(NestOnNewLineWriter)));
	}

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{"Foo":"Bar"}

	)"), FDcCondensedJsonWriter()));

	return true;
};


DC_TEST("DataConfig.Core.JSON.Writer2")
{
	using namespace DcTestJsonDetails;

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{} 

	)"), FDcCondensedJsonWriter()));


	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		[] 

	)"), FDcCondensedJsonWriter()));


	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{} 

	)"), FDcPrettyJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{} 

	)"), FDcPrettyJsonWriter()));


	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		[[[{},{},{}],{},[],{},[[[{}]]]]]

	)"), FDcCondensedJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		[
			[
				[
					{},
					{},
					{}
				],
				{},
				[],
				{},
				[
					[
						[
							{}
						]
					]
				]
			]
		]

	)"), FDcPrettyJsonWriter()));

	return true;
}

DC_TEST("DataConfig.Core.JSON.WriterScalars")
{
	using namespace DcTestJsonDetails;

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		"StringLiteral"

	)"), FDcPrettyJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		null

	)"), FDcPrettyJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		true

	)"), FDcPrettyJsonWriter()));

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		false

	)"), FDcPrettyJsonWriter()));


	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		-1.875

	)"), FDcPrettyJsonWriter()));


	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		"\u0081\u0096\u001F"	

	)"), FDcPrettyJsonWriter()));

	return true;
}

DC_TEST("DataConfig.Core.JSON.WriterStringEscape")
{
	using namespace DcTestJsonDetails;

	UTEST_OK("JsonWriter Roundtrip", RoundtripJsonLiteral(this, TEXT(R"(

		{
			"Escapes1" : "Foo\nBar\r\nDart\t",
			"Escapes2" : "Foo\\Bar\b\fDart\""
		} 

	)"), FDcPrettyJsonWriter()));

	return true;
}

namespace DcTestJsonDetails
{

template<typename TInt, typename TMethod>
static FDcResult CheckJsonReaderNumericCoercion(FAutomationTestBase* Self, TMethod Method, const FString& Str, TInt Expect)
{
	FDcJsonReader Reader(Str);
	FDcCondensedJsonWriter Writer;

	bool bCanCoerce;
	DC_TRY(Reader.Coercion(DcTypeUtils::TDcDataEntryType<TInt>::Value, &bCanCoerce));
	if (!Self->TestTrue("Coercion", bCanCoerce))
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("ReadJson Coersion Fail"));

	TInt Value;
	DC_TRY((Reader.*Method)(&Value));

	if (!Self->TestEqual("Coercion", Value, Expect))
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("ReadJson Coersion Fail"));

	return DcOk();
}

} // namespace DcTestJsonDetails

DC_TEST("DataConfig.Core.JSON.ReaderInts")
{
	using namespace DcTestJsonDetails;

	// ref: https://docs.microsoft.com/en-us/cpp/c-language/cpp-integer-limits?view=msvc-170
	{
		UTEST_OK("JsonReader Coercion int8", CheckJsonReaderNumericCoercion<int8>(this, &FDcJsonReader::ReadInt8, "-128", -128));
		UTEST_OK("JsonReader Coercion int8", CheckJsonReaderNumericCoercion<int8>(this, &FDcJsonReader::ReadInt8, "127", 127));

		UTEST_OK("JsonReader Coercion int16", CheckJsonReaderNumericCoercion<int16>(this, &FDcJsonReader::ReadInt16, "-32768", -32768));
		UTEST_OK("JsonReader Coercion int16", CheckJsonReaderNumericCoercion<int16>(this, &FDcJsonReader::ReadInt16, "32767", 32767));

		UTEST_OK("JsonReader Coercion int32", CheckJsonReaderNumericCoercion<int32>(this, &FDcJsonReader::ReadInt32, "-2147483648", -2147483647 - 1));
		UTEST_OK("JsonReader Coercion int32", CheckJsonReaderNumericCoercion<int32>(this, &FDcJsonReader::ReadInt32, "2147483647", 2147483647));

		UTEST_OK("JsonReader Coercion int64", CheckJsonReaderNumericCoercion<int64>(this, &FDcJsonReader::ReadInt64, "-9223372036854775808", -9223372036854775807ll -1));
		UTEST_OK("JsonReader Coercion int64", CheckJsonReaderNumericCoercion<int64>(this, &FDcJsonReader::ReadInt64, "9223372036854775807", 9223372036854775807ll));
	}

	{
		UTEST_OK("JsonReader Coercion int8", CheckJsonReaderNumericCoercion<uint8>(this, &FDcJsonReader::ReadUInt8, "255", 255u));
		UTEST_OK("JsonReader Coercion int16", CheckJsonReaderNumericCoercion<uint16>(this, &FDcJsonReader::ReadUInt16, "65535 ", 65535u));
		UTEST_OK("JsonReader Coercion int32", CheckJsonReaderNumericCoercion<uint32>(this, &FDcJsonReader::ReadUInt32, "4294967295 ", 4294967295u));
		UTEST_OK("JsonReader Coercion int64", CheckJsonReaderNumericCoercion<uint64>(this, &FDcJsonReader::ReadUInt64, "18446744073709551615", 18446744073709551615ull));
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.ReaderFloats")
{
	using namespace DcTestJsonDetails;

	// ref: https://github.com/nlohmann/json/blob/develop/test/src/unit-testsuites.cpp
	{
		auto _TestFloat = [this](FString Str, float Expect)
		{
			return CheckJsonReaderNumericCoercion<float>(this, &FDcJsonReader::ReadFloat, Str, Expect);
		};

		UTEST_OK("JsonReader Coercion float", _TestFloat("0.0", 0.0f));
		UTEST_OK("JsonReader Coercion float", _TestFloat("-0.0", 0.0f));
		UTEST_OK("JsonReader Coercion float", _TestFloat("1.0", 1.0));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1.0", -1.0));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1.5", 1.5));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1.5", -1.5));
        UTEST_OK("JsonReader Coercion float", _TestFloat("3.1416", 3.1416));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1E10", 1E10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1e10", 1e10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1E+10", 1E+10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1E-10", 1E-10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1E10", -1E10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1e10", -1e10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1E+10", -1E+10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("-1E-10", -1E-10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1.234E+10", 1.234E+10));
        UTEST_OK("JsonReader Coercion float", _TestFloat("1.234E-10", 1.234E-10));

	}

	{
		auto _TestDouble = [this](FString Str, double Expect)
		{
			return CheckJsonReaderNumericCoercion<double>(this, &FDcJsonReader::ReadDouble, Str, Expect);
		};

		UTEST_OK("JsonReader Coercion double", _TestDouble("0.0", 0.0f));
		UTEST_OK("JsonReader Coercion double", _TestDouble("-0.0", 0.0f));
		UTEST_OK("JsonReader Coercion double", _TestDouble("1.0", 1.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1.0", -1.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1.5", 1.5));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1.5", -1.5));
        UTEST_OK("JsonReader Coercion double", _TestDouble("3.1416", 3.1416));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1E10", 1E10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1e10", 1e10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1E+10", 1E+10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1E-10", 1E-10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1E10", -1E10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1e10", -1e10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1E+10", -1E+10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1E-10", -1E-10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1.234E+10", 1.234E+10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1.234E-10", 1.234E-10));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1.79769e+308", 1.79769e+308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("2.22507e-308", 2.22507e-308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-1.79769e+308", -1.79769e+308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-2.22507e-308", -2.22507e-308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("4.9406564584124654e-324", 4.9406564584124654e-324));
        UTEST_OK("JsonReader Coercion double", _TestDouble("2.2250738585072009e-308]", 2.2250738585072009e-308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("2.2250738585072014e-308]", 2.2250738585072014e-308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1.7976931348623157e+308]", 1.7976931348623157e+308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1e-10000", 0.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("18446744073709551616", 18446744073709551616.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("-9223372036854775809",-9223372036854775809.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("0.9868011474609375",0.9868011474609375));
        UTEST_OK("JsonReader Coercion double", _TestDouble("123e34", 123e34));
        UTEST_OK("JsonReader Coercion double", _TestDouble("45913141877270640000.0", 45913141877270640000.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("2.2250738585072011e-308", 2.2250738585072011e-308));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1e-214748363", 0.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1e-214748364", 0.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("0.017976931348623157e+310", 1.7976931348623157e+308));

        UTEST_OK("JsonReader Coercion double", _TestDouble("72057594037927928.0", 72057594037927928.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("72057594037927936.0", 72057594037927936.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("72057594037927932.0", 72057594037927936.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("7205759403792793199999e-5", 72057594037927928.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("7205759403792793200001e-5", 72057594037927936.0));

        UTEST_OK("JsonReader Coercion double", _TestDouble("9223372036854774784.0", 9223372036854774784.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("9223372036854775808.0", 9223372036854775808.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("9223372036854775296.0", 9223372036854775808.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("922337203685477529599999e-5", 9223372036854774784.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("922337203685477529600001e-5", 9223372036854775808.0));

        UTEST_OK("JsonReader Coercion double", _TestDouble("10141204801825834086073718800384", 10141204801825834086073718800384.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("10141204801825835211973625643008", 10141204801825835211973625643008.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("10141204801825834649023672221696", 10141204801825835211973625643008.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1014120480182583464902367222169599999e-5", 10141204801825834086073718800384.0));
        UTEST_OK("JsonReader Coercion double", _TestDouble("1014120480182583464902367222169600001e-5", 10141204801825835211973625643008.0));

	}

	return true;
}


namespace DcTestJsonDetails
{

static FDcResult CheckJsonScalarRoundtrip(FAutomationTestBase* Self, const FString& Str, EDcDataEntry Entry)
{
	FDcJsonReader Reader(Str);
	FDcCondensedJsonWriter Writer;

	DC_TRY(DcSerDeUtils::DispatchPipeVisit(Entry, &Reader, &Writer));

	if (!Self->TestEqual("JsonScalar Roundtrip", Str, Writer.Sb.ToString()))
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("JsonScalar Roundtrip Fail"));

	return DcOk();
}

} // namespace DcTestJsonDetails


DC_TEST("DataConfig.Core.JSON.IntRoundtrips")
{
	using namespace DcTestJsonDetails;

	{
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "-128", EDcDataEntry::Int8));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "127", EDcDataEntry::Int8));

		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "-32768", EDcDataEntry::Int16));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "32767", EDcDataEntry::Int16));

		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "-2147483648", EDcDataEntry::Int32));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "2147483647", EDcDataEntry::Int32));

		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "-9223372036854775808", EDcDataEntry::Int64));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "9223372036854775807", EDcDataEntry::Int64));
	}

	{
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "255", EDcDataEntry::UInt8));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "65535", EDcDataEntry::UInt16));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "4294967295", EDcDataEntry::UInt32));
		UTEST_OK("JsonWriter IntRoundtrips", CheckJsonScalarRoundtrip(this, "18446744073709551615", EDcDataEntry::UInt64));
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.FloatsRoundtrips")
{
	using namespace DcTestJsonDetails;

	{
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "0", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "-1", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1.5", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "-1.5", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1.234E+10", EDcDataEntry::Float));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1.234E-10", EDcDataEntry::Float));
	}

	{
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "0", EDcDataEntry::Double));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "-1", EDcDataEntry::Double));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1", EDcDataEntry::Double));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "1.5", EDcDataEntry::Double));
		UTEST_OK("JsonWriter FloatRoundtrips", CheckJsonScalarRoundtrip(this, "-1.5", EDcDataEntry::Double));
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.WriteRawString")
{
	using namespace DcTestJsonDetails;

	FDcCondensedJsonWriter Writer;
	UTEST_OK("Json WriteRawString", [&]{
		DC_TRY(Writer.WriteMapRoot());
		DC_TRY(Writer.WriteString("Foo"));
		DC_TRY(Writer.WriteRawStringValue("\"Bar\""));
		DC_TRY(Writer.WriteMapEnd());
		return DcOk();
	}());

	UTEST_EQUAL("Json WriteRawString", Writer.Sb.ToString(), TEXT(R"({"Foo":"Bar"})"));
	return true;
}


DC_TEST("DataConfig.Core.JSON.TextNameCoercion")
{
	using namespace DcTestJsonDetails;

	UTEST_OK("JsonWriter TextNameCoercion", CheckJsonScalarRoundtrip(this, TEXT(R"("Hello")"), EDcDataEntry::String));
	UTEST_OK("JsonWriter TextNameCoercion", CheckJsonScalarRoundtrip(this, TEXT(R"("Hello")"), EDcDataEntry::Name));
	UTEST_OK("JsonWriter TextNameCoercion", CheckJsonScalarRoundtrip(this, TEXT(R"("Hello")"), EDcDataEntry::Text));

	UTEST_OK("JsonWriter TextNameCoercion", CheckJsonScalarRoundtrip(this, FString::Printf(TEXT(R"("%s")"), *FString::ChrN(1023, TCHAR('X'))), EDcDataEntry::Name));
	UTEST_DIAG("JsonWriter TextNameCoercion",
		CheckJsonScalarRoundtrip(this, FString::Printf(TEXT(R"("%s")"), *FString::ChrN(1024, TCHAR('X'))), EDcDataEntry::Name),
		DcDReadWrite, FNameOverSize
		);

	return true;
}

DC_TEST("DataConfig.Core.JSON.UTF8RoundTrips")
{
	TArray<uint8> Buf;
	UTEST_TRUE("JSON UTF8RoundTrips", FFileHelper::LoadFileToArray(Buf, *DcGetFixturePath(TEXT("Fixture_UTF8Roundtrip.json"))));
	Buf.RemoveAll([](uint8 Ch)
	{
		return Ch == '\r';
	});

	FDcAnsiJsonReader Reader;
	UTEST_OK("JSON UTF8RoundTrips", Reader.SetNewString((const char*)Buf.GetData(), Buf.Num()));
	FDcAnsiPrettyJsonWriter Writer;

	FDcPipeVisitor PipeVisitor(&Reader, &Writer);
	UTEST_OK("JSON UTF8RoundTrips", PipeVisitor.PipeVisit());

	int Cmp = TCString<ANSICHAR>::Strncmp((const char*)Buf.GetData(), *Writer.Sb, Buf.Num());
	UTEST_EQUAL("JSON UTF8RoundTrips", Cmp, 0);
	
	return true;
}

