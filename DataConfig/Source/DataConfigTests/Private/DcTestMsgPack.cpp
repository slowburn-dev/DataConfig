#include "DcTestMsgPack.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Diagnostic/DcDiagnosticMsgPack.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackUtils.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"

namespace DcTestMsgPackDetails
{

static FDcResult RoundtripMsgPackFromWriter(FAutomationTestBase* Self, FDcMsgPackWriter&& FromWriter)
{
	FDcMsgPackWriter::BufferType FromBuffer = FromWriter.GetMainBuffer();

	FDcMsgPackReader Reader(FDcBlobViewData{FromBuffer.GetData(), FromBuffer.Num()});
	FDcMsgPackWriter ToWriter;

	FDcScopedDiagHandler ScopedHandler([&](FDcDiagnostic& Diag){
		DcDiagnosticUtils::AmendDiagnostic(Diag, &Reader, &ToWriter);
	});

	FDcPipeVisitor PipeVisitor(&Reader, &ToWriter);
	PipeVisitor.PeekVisit.BindLambda([](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl)
	{
		if (Next == EDcDataEntry::Extension)
		{
			OutControl = EPipeVisitControl::SkipContinue;
			return DcMsgPackUtils::MsgPackExtensionHandler(Visitor->Reader, Visitor->Writer);
		}

		OutControl = EPipeVisitControl::Pass;
		return DcOk();
	});

	DC_TRY(PipeVisitor.PipeVisit());

	FDcMsgPackWriter::BufferType ToBuffer = ToWriter.GetMainBuffer();

	FString FromStr = FBase64::Encode(FromBuffer.GetData(), FromBuffer.Num());
	FString ToStr = FBase64::Encode(ToBuffer.GetData(), ToBuffer.Num());

	if (!Self->TestEqual(TEXT("MsgPack Roundtrip"), ToStr, FromStr))
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("Json Writer Roundtrip Failed: %s -> %s"), *FromStr, *ToStr);

	return DcOk();
}

static FDcResult _TestWriter(FAutomationTestBase* Self, TFunctionRef<FDcResult(FDcMsgPackWriter&)> Fn)
{
	FDcMsgPackWriter Writer;

	{
		FDcScopedDiagHandler ScopedHandler([&](FDcDiagnostic& Diag){
			DcDiagnosticUtils::AmendDiagnostic(Diag, nullptr, &Writer);
		});
		DC_TRY(Fn(Writer));
	}

	DC_TRY(RoundtripMsgPackFromWriter(Self, MoveTemp(Writer)));
	return DcOk();
}

static FDcResult RoundtripJsonMsgpackJson(FAutomationTestBase* Self, FString Str)
{
	FString NormalizedStr = DcAutomationUtils::DcReindentStringLiteral(Str);

	FDcMsgPackWriter::BufferType MsgPackBytes;
	{
		FDcJsonReader Reader(NormalizedStr);
		FDcMsgPackWriter Writer;
		FDcPipeVisitor PipeVisitor(&Reader, &Writer);

		DC_TRY(PipeVisitor.PipeVisit());
		MsgPackBytes = MoveTemp(Writer.GetMainBuffer());
	}

	FString RoundtripJson;
	{
		FDcMsgPackReader Reader({MsgPackBytes.GetData(), MsgPackBytes.Num()});
		FDcJsonWriter Writer;
		FDcPipeVisitor PipeVisitor(&Reader, &Writer);

		DC_TRY(PipeVisitor.PipeVisit());
		//	amend trailing line end
		Writer.Sb << TCHAR('\n');
		RoundtripJson = Writer.Sb.ToString();
	}

	if (!Self->TestEqual(TEXT("Roundtrip Json MsgPack Json"), RoundtripJson, NormalizedStr))
		return DC_FAIL(DcDCommon, CustomMessage) << TEXT("Roundtrip Json MsgPack Json Failed");

	return DcOk();
}

static uint8* _MallocPatternMemory(int Size)
{
	uint8* Bytes = (uint8*)FMemory::Malloc(Size);
	{
		for (int Ix = 0; Ix <= 0xFF; Ix++)
			Bytes[Ix] = (uint8)Ix;
		for (int Ix = 1; Ix <= (Size >> 8); Ix++)
			FMemory::Memcpy(Bytes + (PTRINT)(Ix * (0xFF + 1)), Bytes, 0xFF);
	}
	return Bytes;
}

} // namespace DcTestMsgPackDetails

DC_TEST("DataConfig.Core.MsgPack.Roundtrip1")
{
	using namespace DcTestMsgPackDetails;

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		//	empty
		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteNone());
		DC_TRY(Writer.WriteBool(true));
		DC_TRY(Writer.WriteBool(false));
		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteInt8(TNumericLimits<int8>::Min()));
		DC_TRY(Writer.WriteInt8(TNumericLimits<int8>::Max()));

		DC_TRY(Writer.WriteInt8(0));
		DC_TRY(Writer.WriteInt8(127));
		DC_TRY(Writer.WriteInt8(-32));
		DC_TRY(Writer.WriteInt8(-33));

		DC_TRY(Writer.WriteUInt8(TNumericLimits<uint8>::Min()));
		DC_TRY(Writer.WriteUInt8(TNumericLimits<uint8>::Max()));


		DC_TRY(Writer.WriteUInt8(0));
		DC_TRY(Writer.WriteUInt8(128));

		return DcOk();
	}));


	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteInt16(TNumericLimits<int16>::Min()));
		DC_TRY(Writer.WriteInt16(TNumericLimits<int16>::Max()));

		DC_TRY(Writer.WriteInt32(TNumericLimits<int32>::Min()));
		DC_TRY(Writer.WriteInt32(TNumericLimits<int32>::Max()));

		DC_TRY(Writer.WriteInt64(TNumericLimits<int64>::Min()));
		DC_TRY(Writer.WriteInt64(TNumericLimits<int64>::Max()));

		DC_TRY(Writer.WriteUInt16(TNumericLimits<uint16>::Min()));
		DC_TRY(Writer.WriteUInt16(TNumericLimits<uint16>::Max()));

		DC_TRY(Writer.WriteUInt32(TNumericLimits<uint32>::Min()));
		DC_TRY(Writer.WriteUInt32(TNumericLimits<uint32>::Max()));

		DC_TRY(Writer.WriteUInt64(TNumericLimits<uint64>::Min()));
		DC_TRY(Writer.WriteUInt64(TNumericLimits<uint64>::Max()));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteFloat(0.0f));
		DC_TRY(Writer.WriteFloat(-0.0f));
		DC_TRY(Writer.WriteFloat(1.0f));
		DC_TRY(Writer.WriteFloat(-1.0f));
		DC_TRY(Writer.WriteFloat(1E30));
		DC_TRY(Writer.WriteFloat(1e30));
		DC_TRY(Writer.WriteFloat(-1E10));
		DC_TRY(Writer.WriteFloat(-1e10));
		DC_TRY(Writer.WriteFloat(1.234E+10));
		DC_TRY(Writer.WriteFloat(1.234E-10));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteDouble(0.0f));
		DC_TRY(Writer.WriteDouble(-0.0f));
		DC_TRY(Writer.WriteDouble(1.0f));
		DC_TRY(Writer.WriteDouble(-1.0f));
		DC_TRY(Writer.WriteDouble(1E30));
		DC_TRY(Writer.WriteDouble(1e30));
		DC_TRY(Writer.WriteDouble(-1E10));
		DC_TRY(Writer.WriteDouble(-1e10));
		DC_TRY(Writer.WriteDouble(1.234E+10));
		DC_TRY(Writer.WriteDouble(1.234E-10));

		DC_TRY(Writer.WriteDouble(1.79769e+308));
		DC_TRY(Writer.WriteDouble(2.22507e-308));
		DC_TRY(Writer.WriteDouble(-1.79769e+308));
		DC_TRY(Writer.WriteDouble(-2.22507e-308));
		DC_TRY(Writer.WriteDouble(4.9406564584124654e-324));
		DC_TRY(Writer.WriteDouble(2.2250738585072009e-308));
		DC_TRY(Writer.WriteDouble(2.2250738585072014e-308));
		DC_TRY(Writer.WriteDouble(1.7976931348623157e+308));
		DC_TRY(Writer.WriteDouble(18446744073709551616.0));
		DC_TRY(Writer.WriteDouble(9223372036854775809.0));
		DC_TRY(Writer.WriteDouble(.9868011474609375));
		DC_TRY(Writer.WriteDouble(123e34));
		DC_TRY(Writer.WriteDouble(45913141877270640000.0));
		DC_TRY(Writer.WriteDouble(2.2250738585072011e-308));
		DC_TRY(Writer.WriteDouble(1.7976931348623157e+308));
		DC_TRY(Writer.WriteDouble(72057594037927928.0));
		DC_TRY(Writer.WriteDouble(72057594037927936.0));
		DC_TRY(Writer.WriteDouble(72057594037927936.0));
		DC_TRY(Writer.WriteDouble(72057594037927928.0));
		DC_TRY(Writer.WriteDouble(72057594037927936.0));
		DC_TRY(Writer.WriteDouble(9223372036854774784.0));
		DC_TRY(Writer.WriteDouble(9223372036854775808.0));
		DC_TRY(Writer.WriteDouble(9223372036854775808.0));
		DC_TRY(Writer.WriteDouble(9223372036854774784.0));
		DC_TRY(Writer.WriteDouble(9223372036854775808.0));
		DC_TRY(Writer.WriteDouble(10141204801825834086073718800384.0));
		DC_TRY(Writer.WriteDouble(10141204801825835211973625643008.0));
		DC_TRY(Writer.WriteDouble(10141204801825835211973625643008.0));
		DC_TRY(Writer.WriteDouble(10141204801825834086073718800384.0));
		DC_TRY(Writer.WriteDouble(10141204801825835211973625643008.0));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(TEXT("These are my twisted words.")));
		DC_TRY(Writer.WriteString(TEXT("\u00A2")));
		DC_TRY(Writer.WriteString(TEXT("\xD834\xDD1E")));
		DC_TRY(Writer.WriteString(TEXT("\x0049\x00F1\x0074\x00EB\x0072\x006E\x00E2\x0074\x0069\x00F4\x006E\x00E0\x006C\x0069\x007A\x00E6\x0074\x0069\x00F8\x006E")));

		return DcOk();
	}));

	return true;
}


DC_TEST("DataConfig.Core.MsgPack.Roundtrip_2")
{
	using namespace DcTestMsgPackDetails;

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteArrayRoot());
		DC_TRY(Writer.WriteArrayEnd());

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteArrayRoot());
		for (int Ix = 0; Ix < TNumericLimits<uint16>::Max() - 1; Ix++)
			DC_TRY(Writer.WriteInt32(Ix));
		DC_TRY(Writer.WriteArrayEnd());

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteArrayRoot());
		for (int Ix = 0; Ix < TNumericLimits<uint16>::Max() + 1; Ix++)
			DC_TRY(Writer.WriteInt32(Ix));
		DC_TRY(Writer.WriteArrayEnd());

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteMapRoot());
		DC_TRY(Writer.WriteMapEnd());

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteMapRoot());
		for (int Ix = 0; Ix < TNumericLimits<uint16>::Max() - 1; Ix++)
		{
			DC_TRY(Writer.WriteInt32(Ix));
			DC_TRY(Writer.WriteInt32(Ix));
		}
		DC_TRY(Writer.WriteMapEnd());

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip2", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteMapRoot());
		for (int Ix = 0; Ix < TNumericLimits<uint16>::Max() + 1; Ix++)
		{
			DC_TRY(Writer.WriteInt32(Ix));
			DC_TRY(Writer.WriteInt32(Ix));
		}
		DC_TRY(Writer.WriteMapEnd());

		return DcOk();
	}));

	return true;
}

DC_TEST("DataConfig.Core.MsgPack.String_8_16_32")
{
	using namespace DcTestMsgPackDetails;

	auto _RepeatString = [](const FString& Str, int N)
	{
		FString Out;
		for (int Ix = 0; Ix < N; Ix++)
			Out += Str;
		return Out;
	};

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(TEXT("")));

		return DcOk();
	}));

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(TEXT("1234567890ABCDEF")));

		return DcOk();
	}));

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(_RepeatString(TEXT("1234567890ABCDEF"), (0xFF + 1) / 16)));

		return DcOk();
	}));

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(_RepeatString(TEXT("1234567890ABCDEF"), ((0xFF +1 ) / 16) + 1)));

		return DcOk();
	}));

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(_RepeatString(TEXT("1234567890ABCDEF"), ((0xFFFF +1 ) / 16))));

		return DcOk();
	}));

	UTEST_OK("MsgPack String_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteString(_RepeatString(TEXT("1234567890ABCDEF"), ((0xFFFF +1 ) / 16) + 1)));

		return DcOk();
	}));

	return true;
}

DC_TEST("DataConfig.Core.MsgPack.Blob_8_16_32")
{
	using namespace DcTestMsgPackDetails;

	int Size = 0x1FFFF;
	uint8* Bytes = _MallocPatternMemory(Size);
	ON_SCOPE_EXIT
	{
		FMemory::Free(Bytes);
	};

	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 0}));

		return DcOk();
	}));

	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 1}));

		return DcOk();
	}));


	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 0xFF}));

		return DcOk();
	}));

	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 0xFF + 1}));

		return DcOk();
	}));

	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 0xFFFF}));

		return DcOk();
	}));

	UTEST_OK("MsgPack Blob_8_16_32", _TestWriter(this, [&](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteBlob({Bytes, 0xFFFF + 1}));

		return DcOk();
	}));

	return true;
}


DC_TEST("DataConfig.Core.MsgPack.RoundtripJsonMsgpackJson")
{
	using namespace DcTestMsgPackDetails;

	UTEST_OK("MsgPack RoundtripJsonMsgpackJson", RoundtripJsonMsgpackJson(this, TEXT(R"(

		[]

	)")));

	UTEST_OK("MsgPack RoundtripJsonMsgpackJson", RoundtripJsonMsgpackJson(this, TEXT(R"(

		{}

	)")));

	UTEST_OK("MsgPack RoundtripJsonMsgpackJson", RoundtripJsonMsgpackJson(this, TEXT(R"(

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

	)")));


	UTEST_OK("MsgPack RoundtripJsonMsgpackJson", RoundtripJsonMsgpackJson(this, TEXT(R"(

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

	)")));

	return true;
}

DC_TEST("DataConfig.Core.MsgPack.Extension")
{
	using namespace DcTestMsgPackDetails;

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteFixExt1(1, 2));
		DC_TRY(Writer.WriteFixExt2(3, {{4, 5}}));
		DC_TRY(Writer.WriteFixExt4(6, {{7, 8, 9, 10}}));
		DC_TRY(Writer.WriteFixExt8(11, {{12, 13, 14, 15, 16, 17, 18, 19}}));
		DC_TRY(Writer.WriteFixExt16(20, {{
				21, 22, 23, 24, 25, 26, 27, 28,
				29, 30, 31, 32, 33, 34, 35, 36,
			}}));

		return DcOk();
	}));

	int Size = 0x1FFFF;
	uint8* Bytes = _MallocPatternMemory(Size);
	ON_SCOPE_EXIT
	{
		FMemory::Free(Bytes);
	};

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(1, {Bytes, 0} ));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(2, {Bytes, 1} ));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(3, {Bytes, 0xFF} ));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(4, {Bytes, 0xFF + 1} ));

		return DcOk();
	}));

	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(5, {Bytes, 0xFFFF} ));

		return DcOk();
	}));


	UTEST_OK("MsgPack Roundtrip1", _TestWriter(this, [Bytes](FDcMsgPackWriter& Writer)
	{
		DC_TRY(Writer.WriteExt(6, {Bytes, 0xFFFF + 1} ));

		return DcOk();
	}));

	return true;
}

DC_TEST("DataConfig.Core.MsgPack.TextName")
{
	using namespace DcTestMsgPackDetails;

	{
		FDcMsgPackWriter Writer;
		UTEST_OK("MsgPack Name", Writer.WriteName(FName(TEXT("Foo1"))));
		UTEST_OK("MsgPack Name", Writer.WriteText(FText::FromString(TEXT("Foo2"))));

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader({Buffer.GetData(), Buffer.Num()});

		bool bCanCoercion;
		UTEST_OK("MsgPack Name", Reader.Coercion(EDcDataEntry::Name, &bCanCoercion));
		UTEST_TRUE("MsgPack Name", bCanCoercion);
		FName Name1;
		UTEST_OK("MsgPack Name", Reader.ReadName(&Name1));

		FText Text1;
		UTEST_OK("MsgPack Name", Reader.Coercion(EDcDataEntry::Text, &bCanCoercion));
		UTEST_TRUE("MsgPack Name", bCanCoercion);
		UTEST_OK("MsgPack Name", Reader.ReadText(&Text1));

		UTEST_EQUAL("MsgPack Name", Name1, FName(TEXT("Foo1")));
		UTEST_EQUAL("MsgPack Name", Text1.ToString(), (TEXT("Foo2")));
	}

	{
		FDcMsgPackWriter Writer;
		UTEST_OK("MsgPack Name", Writer.WriteString(FString::ChrN(1024, TCHAR('X'))));

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader({Buffer.GetData(), Buffer.Num()});

		FName Name1;
		UTEST_DIAG("MsgPack Name", Reader.ReadName(&Name1), DcDReadWrite, FNameOverSize);
	}

	{
		FDcMsgPackWriter Writer;
		UTEST_OK("MsgPack Name", Writer.WriteString(FString::ChrN(1023, TCHAR('X'))));

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader({Buffer.GetData(), Buffer.Num()});

		FName Name1;
		UTEST_OK("MsgPack Name", Reader.ReadName(&Name1));

		UTEST_EQUAL("MsgPack Name", Name1.ToString(), FString::ChrN(1023, TCHAR('X')))
	}

	return true;
}


DC_TEST("DataConfig.Core.MsgPack.Diags")
{
	using namespace DcTestMsgPackDetails;

	{
		FDcMsgPackWriter Writer;

		UTEST_OK("MsgPack Diags", Writer.WriteArrayRoot());
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(1));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(2));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(3));
		UTEST_OK("MsgPack Diags", Writer.WriteArrayEnd());

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader(FDcBlobViewData{Buffer.GetData(), Buffer.Num()});
		UTEST_OK("MsgPack Diags", Reader.ReadArrayRoot());
		int32 Value;
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_DIAG("MsgPack Diags", Reader.ReadArrayEnd(), DcDMsgPack, ArrayRemains);
	}

	{
		FDcMsgPackWriter Writer;

		UTEST_OK("MsgPack Diags", Writer.WriteArrayRoot());
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(1));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(2));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(3));
		UTEST_OK("MsgPack Diags", Writer.WriteArrayEnd());
		UTEST_OK("MsgPack Diags", Writer.WriteString(TEXT("Long Ending Pad")));


		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader(FDcBlobViewData{Buffer.GetData(), Buffer.Num()});
		UTEST_OK("MsgPack Diags", Reader.ReadArrayRoot());
		int32 Value;
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_DIAG("MsgPack Diags", Reader.ReadInt32(&Value), DcDMsgPack, ReadPassArrayEnd);
	}

	{
		FDcMsgPackWriter Writer;

		UTEST_OK("MsgPack Diags", Writer.WriteMapRoot());
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(1));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(2));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(3));
		UTEST_DIAG("MsgPack Diags", Writer.WriteMapEnd(), DcDMsgPack, UnexpectedMapEnd);
	}


	{
		FDcMsgPackWriter Writer;

		UTEST_OK("MsgPack Diags", Writer.WriteMapRoot());
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(1));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(2));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(3));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(4));
		UTEST_OK("MsgPack Diags", Writer.WriteMapEnd());

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader(FDcBlobViewData{Buffer.GetData(), Buffer.Num()});

		UTEST_OK("MsgPack Diags", Reader.ReadMapRoot());
		int32 Value;
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_DIAG("MsgPack Diags", Reader.ReadMapEnd(), DcDMsgPack, MapRemains);
	}

	{
		FDcMsgPackWriter Writer;

		UTEST_OK("MsgPack Diags", Writer.WriteMapRoot());
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(1));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(2));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(3));
		UTEST_OK("MsgPack Diags", Writer.WriteInt32(4));
		UTEST_OK("MsgPack Diags", Writer.WriteMapEnd());

		auto& Buffer = Writer.GetMainBuffer();
		FDcMsgPackReader Reader(FDcBlobViewData{Buffer.GetData(), Buffer.Num()});

		UTEST_OK("MsgPack Diags", Reader.ReadMapRoot());
		int32 Value;
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_OK("MsgPack Diags", Reader.ReadInt32(&Value));
		UTEST_DIAG("MsgPack Diags", Reader.ReadInt32(&Value), DcDMsgPack, ReadPassMapEnd);
	}

	return true;
}

DC_TEST("DataConfig.Core.MsgPack.FormatHighlight")
{
	FDcMsgPackWriter Writer;

	{
		UTEST_OK("MsgPack FormatHightlight", Writer.WriteInt16(123));
		FDcDiagnostic DiagWriter({DcDCommon::Category, DcDCommon::CustomMessage});
		Writer.FormatDiagnostic(DiagWriter);
		UTEST_EQUAL("MsgPack FormatHightlight", DiagWriter.Highlights[0].Formatted, TEXT("Last write: Int16"));

	}

	{
		FDcMsgPackReader Reader(FDcBlobViewData::From(Writer.GetMainBuffer()));
		UTEST_OK("MsgPack FormatHightlight", Reader.ReadInt16(nullptr));

		FDcDiagnostic DiagReader({DcDCommon::Category, DcDCommon::CustomMessage});
		Reader.FormatDiagnostic(DiagReader);
		UTEST_EQUAL("MsgPack FormatHightlight", DiagReader.Highlights[0].Formatted, TEXT("Last read: Int16"));
	}

	return true;
}

namespace DcTestMsgPackDetails
{

static void _ParseBytes(const FString& Str, TArray<uint8>& Bytes)
{
	const TCHAR* Head = GetData(Str);
	int Ix = 0;
	int Len = Str.Len();

	while (Ix < Len)
	{
		int Hex1 = FParse::HexDigit(Head[Ix]);
		int Hex2 = FParse::HexDigit(Head[Ix+1]);
		Bytes.Add(Hex1 * 16 + Hex2);

		Ix += 3; // include separator
	}
}

static FDcResult HandlerMsgPackTestBlob(FDcDeserializeContext& Ctx)
{
	FString Str;
	DC_TRY(Ctx.Reader->ReadString(&Str));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FDcTestMsgPackBlob* Blob = (FDcTestMsgPackBlob*)Datum.DataPtr;
	_ParseBytes(Str, Blob->Blob);

	return DcOk();
}

static FDcResult HandlerMsgPackTestSuite(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Writer->WriteStructRoot());
	DC_TRY(Ctx.Writer->WriteName(TEXT("Groups")));
	DC_TRY(Ctx.Writer->WriteArrayRoot());
	DC_TRY(Ctx.Reader->ReadMapRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::MapEnd)
			break;

		FString Name;
		DC_TRY(Ctx.Reader->ReadString(&Name));

		FDcPropertyDatum Datum;
		DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

		{
			FMsgPackTestGroup* Group = (FMsgPackTestGroup*)Datum.DataPtr;
			Group->Name = Name;

			Ctx.Properties.Push(Datum.Property);
			DC_TRY(Ctx.Writer->PushTopStructPropertyState(Datum, Datum.Property.GetFName()));
			DC_TRY(Ctx.Deserializer->Deserialize(Ctx));
			Ctx.Properties.Pop();
		}
	}

	DC_TRY(Ctx.Writer->WriteArrayEnd());
	DC_TRY(Ctx.Writer->WriteStructEnd());
	DC_TRY(Ctx.Reader->ReadMapEnd());
	return DcOk();
}

static FDcResult HandlerMsgPackTestGroup(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Writer->WriteStructRoot());
	DC_TRY(Ctx.Writer->WriteName(TEXT("Fixtures")));

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	DC_TRY(Ctx.Writer->WriteArrayRoot());
	while (true)
	{
		EDcDataEntry Next;
		DC_TRY(Ctx.Reader->PeekRead(&Next));
		if (Next == EDcDataEntry::ArrayEnd)
			break;

		DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
	}

	DC_TRY(Ctx.Reader->ReadArrayEnd());
	DC_TRY(Ctx.Writer->WriteArrayEnd());
	DC_TRY(Ctx.Writer->WriteStructEnd());
	return DcOk();
}

static FDcResult HandlerMsgPackFixture(FDcDeserializeContext& Ctx)
{
	DC_TRY(Ctx.Reader->ReadMapRoot());
	DC_TRY(Ctx.Writer->WriteStructRoot());

	auto _WriteActual = [&Ctx](FDcMsgPackWriter& MsgPackWriter, EDcTestMsgPackCompareType Type) -> FDcResult
	{
		DC_TRY(Ctx.Writer->WriteName(TEXT("Type")));
		DC_TRY(Ctx.Writer->WriteEnumField(Type));

		auto& MainBuffer = MsgPackWriter.GetMainBuffer();
		DC_TRY(Ctx.Writer->WriteName(TEXT("Expect")));
		DC_TRY(Ctx.Writer->WriteBlob(FDcBlobViewData{
			MainBuffer.GetData(),
			MainBuffer.Num()
		}));

		return DcOk();
	};

	while (true)
	{
		EDcDataEntry Next;
		DC_TRY(Ctx.Reader->PeekRead(&Next));

		if (Next == EDcDataEntry::MapEnd)
			break;

		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));

		if (Str == TEXT("msgpack"))
		{
			DC_TRY(Ctx.Writer->WriteName(TEXT("Actuals")));
			DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
		}
		else if (Str == TEXT("nil"))
		{
			FDcMsgPackWriter MsgPackWriter;

			DC_TRY(Ctx.Reader->ReadNone());
			DC_TRY(MsgPackWriter.WriteNone());

			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
		else if (Str == TEXT("bool"))
		{
			FDcMsgPackWriter MsgPackWriter;

			bool Value;
			DC_TRY(Ctx.Reader->ReadBool(&Value));
			DC_TRY(MsgPackWriter.WriteBool(Value));

			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
		else if (Str == TEXT("string"))
		{
			FDcMsgPackWriter MsgPackWriter;

			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));
			DC_TRY(MsgPackWriter.WriteString(Value));

			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
		else if (Str == TEXT("binary"))
		{
			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));

			TArray<uint8> Bytes;
			_ParseBytes(Value, Bytes);

			FDcMsgPackWriter MsgPackWriter;
			DC_TRY(MsgPackWriter.WriteBlob(FDcBlobViewData::From(Bytes)));

			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
		else if (Str == TEXT("number"))
		{
			bool bReadAsStr;
			FString Value;
			DC_TRY(Ctx.Reader->Coercion(EDcDataEntry::String, &bReadAsStr));
			check(bReadAsStr);
			DC_TRY(Ctx.Reader->ReadString(&Value));

			DC_TRY(Ctx.Writer->WriteName(TEXT("ExpectDoubleStr")));
			DC_TRY(Ctx.Writer->WriteString(Value));

			DC_TRY(Ctx.Writer->WriteName(TEXT("Type")));
			DC_TRY(Ctx.Writer->WriteEnumField(EDcTestMsgPackCompareType::NumberBigNumber));

		}
		else if (Str == TEXT("bignum"))
		{
			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));

			DC_TRY(Ctx.Writer->WriteName(TEXT("ExpectDoubleStr")));
			DC_TRY(Ctx.Writer->WriteString(Value));

			DC_TRY(Ctx.Writer->WriteName(TEXT("Type")));
			DC_TRY(Ctx.Writer->WriteEnumField(EDcTestMsgPackCompareType::NumberBigNumber));
		}
		else if (Str == TEXT("ext"))
		{
			DC_TRY(Ctx.Reader->ReadArrayRoot());

			uint8 ExtType;
			DC_TRY(Ctx.Reader->ReadUInt8(&ExtType));

			FString BytesStr;
			DC_TRY(Ctx.Reader->ReadString(&BytesStr));
			TArray<uint8> Bytes;
			_ParseBytes(BytesStr, Bytes);

			DC_TRY(Ctx.Reader->ReadArrayEnd());

			FDcMsgPackWriter MsgPackWriter;
			if (ExtType < 6)	// match test suite setup
			{
				if (Bytes.Num() <= 1)
				{
					DC_TRY(MsgPackWriter.WriteFixExt1(ExtType, Bytes[0]));
				}
				else if (Bytes.Num() <= 2)
				{
					DC_TRY(MsgPackWriter.WriteFixExt2(ExtType, FDcBytes2::From(Bytes)));
				}
				else if (Bytes.Num() <= 4)
				{
					DC_TRY(MsgPackWriter.WriteFixExt4(ExtType, FDcBytes4::From(Bytes)));
				}
				else if (Bytes.Num() <= 8)
				{
					DC_TRY(MsgPackWriter.WriteFixExt8(ExtType, FDcBytes8::From(Bytes)));
				}
				else if (Bytes.Num() <= 16)
				{
					DC_TRY(MsgPackWriter.WriteFixExt16(ExtType, FDcBytes16::From(Bytes)));
				}
				else
				{
					checkNoEntry();
				}
			}
			else
			{
				DC_TRY(MsgPackWriter.WriteExt(ExtType, FDcBlobViewData::From(Bytes)));
			}

			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::Extension));
		}
		else if (Str == TEXT("array"))
		{
			int ArrayCount = 1;
			DC_TRY(Ctx.Reader->ReadArrayRoot());
			FDcMsgPackWriter MsgPackWriter;
			DC_TRY(MsgPackWriter.WriteArrayRoot());

			FDcPipeVisitor Visitor(Ctx.Reader, &MsgPackWriter);
			Visitor.PeekVisit.BindLambda(
			[](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl) -> FDcResult {
				if (Next == EDcDataEntry::Double)
				{
					//	coerce into int8 to match with fixture
					int8 Value;
					DC_TRY(Visitor->Reader->ReadInt8(&Value));
					DC_TRY(Visitor->Writer->WriteInt8(Value));
					OutControl = EPipeVisitControl::SkipContinue;
				}
				return DcOk();
			});

			Visitor.PostVisit.BindLambda(
			[&ArrayCount](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl) -> FDcResult {
				if (Next == EDcDataEntry::ArrayEnd)
					ArrayCount--;
				if (Next == EDcDataEntry::ArrayRoot)
					ArrayCount++;

				if (ArrayCount == 0)
					OutControl = EPipeVisitControl::BreakVisit;
				return DcOk();
			});

			DC_TRY(Visitor.PipeVisit());
			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
		else if (Str == TEXT("map"))
		{
			int MapCount = 1;
			DC_TRY(Ctx.Reader->ReadMapRoot());
			FDcMsgPackWriter MsgPackWriter;
			DC_TRY(MsgPackWriter.WriteMapRoot());

			FDcPipeVisitor Visitor(Ctx.Reader, &MsgPackWriter);
			Visitor.PeekVisit.BindLambda(
			[](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl) -> FDcResult {
				if (Next == EDcDataEntry::Double)
				{
					//	coerce into int8 to match with fixture
					int8 Value;
					DC_TRY(Visitor->Reader->ReadInt8(&Value));
					DC_TRY(Visitor->Writer->WriteInt8(Value));
					OutControl = EPipeVisitControl::SkipContinue;
				}
				return DcOk();
			});

			Visitor.PostVisit.BindLambda(
			[&MapCount](FDcPipeVisitor* Visitor, EDcDataEntry Next, EPipeVisitControl& OutControl) -> FDcResult {
				if (Next == EDcDataEntry::MapEnd)
					MapCount--;
				if (Next == EDcDataEntry::MapRoot)
					MapCount++;

				if (MapCount == 0)
					OutControl = EPipeVisitControl::BreakVisit;
				return DcOk();
			});

			DC_TRY(Visitor.PipeVisit());
			DC_TRY(_WriteActual(MsgPackWriter, EDcTestMsgPackCompareType::PrettyPrintEqualable));
		}
	}

	DC_TRY(Ctx.Reader->ReadMapEnd());
	DC_TRY(Ctx.Writer->WriteStructEnd());

	return DcOk();
}


} // namespace DcTestMsgPackDetails


DC_TEST("DataConfig.Core.MsgPack.TestSuite")
{
	using namespace DcTestMsgPackDetails;

	TArray<uint8> Buf;
	verify(FFileHelper::LoadFileToArray(Buf, *DcGetFixturePath(TEXT("MsgPack/msgpack-test-suite.json"))));

	FDcAnsiJsonReader Reader((char*)Buf.GetData(), Buf.Num());
	FMsgPackTestSuite TestSuite;

	UTEST_OK("MsgPack TestSuite", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&TestSuite), [](FDcDeserializeContext& Ctx)
	{
		Ctx.Deserializer->AddStructHandler(TBaseStructure<FMsgPackTestSuite>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerMsgPackTestSuite));
		Ctx.Deserializer->AddStructHandler(TBaseStructure<FMsgPackTestGroup>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerMsgPackTestGroup));
		Ctx.Deserializer->AddStructHandler(TBaseStructure<FMsgPackTestFixture>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerMsgPackFixture));
		Ctx.Deserializer->AddStructHandler(TBaseStructure<FDcTestMsgPackBlob>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerMsgPackTestBlob));

	}, DcAutomationUtils::EDefaultSetupType::SetupJSONHandlers));

	for (auto& Group : TestSuite.Groups)
	{
		for (auto& Fixture : Group.Fixtures)
		{
			if (Fixture.Type == EDcTestMsgPackCompareType::Skip)
				continue;

			if (Fixture.Type == EDcTestMsgPackCompareType::PrettyPrintEqualable)
			{
				FDcMsgPackReader ExpectReader{FDcBlobViewData::From(Fixture.Expect)};
				FString ExpectStr = DcAutomationUtils::DumpFormat(&ExpectReader);

				for (auto& Actual : Fixture.Actuals)
				{
					FDcMsgPackReader ActualReader{FDcBlobViewData::From(Actual.Blob)};
					FString ActualStr = DcAutomationUtils::DumpFormat(&ActualReader);

					UTEST_EQUAL(
						*FString::Printf(TEXT("MsgPack TestSuite: %s"), *Group.Name),
						TEXT("\n") + ActualStr + TEXT("\n"),
						TEXT("\n") + ExpectStr + TEXT("\n")
					);
				}
			}
			else if (Fixture.Type == EDcTestMsgPackCompareType::NumberBigNumber)
			{
				for (auto& Actual : Fixture.Actuals)
				{
					FDcMsgPackReader ActualReader(FDcBlobViewData::From(Actual.Blob));

					FString ActualNumericStr;
					UTEST_OK("MsgPack TestSuite", DcAutomationUtils::DumpNextNumericAsString(&ActualReader, &ActualNumericStr));

					UTEST_EQUAL(
						*FString::Printf(TEXT("MsgPack TestSuite: %s"), *Group.Name),
						 ActualNumericStr,
						 Fixture.ExpectDoubleStr
					);
				}
			}
			else if (Fixture.Type == EDcTestMsgPackCompareType::Extension)
			{
				FDcMsgPackReader ExpectReader{FDcBlobViewData::From(Fixture.Expect)};

				uint8 ExpectType;
				TArray<uint8> ExpectBytes;
				UTEST_OK("MsgPack TestSuite", DcMsgPackUtils::ReadExtBytes(&ExpectReader, ExpectType, ExpectBytes));

				FString ExpectBase64Str = FBase64::Encode(ExpectBytes);

				for (auto& Actual : Fixture.Actuals)
				{
					FDcMsgPackReader ActualReader(FDcBlobViewData::From(Actual.Blob));

					uint8 ActualType;
					TArray<uint8> ActualBytes;
					UTEST_OK("MsgPack TestSuite", DcMsgPackUtils::ReadExtBytes(&ActualReader, ActualType, ActualBytes));

					FString ActualBase64Str = FBase64::Encode(ActualBytes);

					UTEST_EQUAL(
						*FString::Printf(TEXT("MsgPack TestSuite: %s"), *Group.Name),
						 ActualType,
						 ExpectType
					);
					UTEST_EQUAL(
						*FString::Printf(TEXT("MsgPack TestSuite: %s"), *Group.Name),
						 ActualBase64Str,
						 ExpectBase64Str
					);
				}
			}
		}
	}

	return true;
}

