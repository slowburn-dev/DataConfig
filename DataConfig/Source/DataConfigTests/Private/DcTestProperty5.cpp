#include "DcTestProperty5.h"

#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

#include "Internationalization/StringTableRegistry.h"

void FDcTestStructWithText::MakeFixture()
{
	// text test fixtures
	if (!FStringTableRegistry::Get().FindStringTable("/Bogus/Path/To/Table"))
	{
		LOCTABLE_NEW("/Bogus/Path/To/Table", "DataConfig.Tests");
		LOCTABLE_SETSTRING("/Bogus/Path/To/Table", "FOO_TEST", "Localize String Foo");
	}

	TextFieldAlpha = NSLOCTEXT("DataConfig.Tests", "BAR_TEST", "Localize String Bar");
	TextFieldBeta = FText::FromStringTable(FName(TEXT("/Bogus/Path/To/Table")), TEXT("FOO_TEST"));
}

DC_TEST("DataConfig.Core.Property.Defaults")
{
	FString Str = TEXT(R"(
		{
			"StrFieldWithDefault" : "Bar",
			"IntFieldWithDefault" : 123,

			"StrArrFieldWithDefault" : ["One", "Two", "Three", "Four"],
			"StrSetFieldWithDefault" : ["Foo", "Bar"],
			"StringIntMapFieldWithDefault" : {
				"Five" : 5,
				"Ten" : 10
			}
		}
	)");

	FDcJsonReader Reader(Str);
	FDcTestStructWithDefaults Dest;

	UTEST_OK("Deserialize With Defaults", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest)));

	FDcTestStructWithDefaults Expect;
	Expect.StrFieldWithDefault = "Bar";
	Expect.IntFieldWithDefault = 123;

	Expect.StrArrFieldWithDefault = {"One", "Two", "Three", "Four"};
	Expect.StrSetFieldWithDefault = {"Foo", "Bar"};
	Expect.StringIntMapFieldWithDefault = {
		{"Five", 5},
		{"Ten", 10},
	};

	UTEST_OK("Deserialize With Defaults", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Dest), FDcPropertyDatum(&Expect)));

	return true;
}

DC_TEST("DataConfig.Core.Property.Text")
{
	FString Str = TEXT(R"-(

		{
			"TextFieldAlpha" : "NSLOCTEXT(\"DataConfig.Tests\", \"BAR_TEST\", \"Localize String Bar\")",
			"TextFieldBeta" : "LOCTABLE(\"/Bogus/Path/To/Table\", \"FOO_TEST\")"
		}

	)-");


	FDcJsonReader Reader(Str);
	FDcTestStructWithText Dest;

	FDcTestStructWithText Expect;
	Expect.MakeFixture();

	UTEST_OK("Deserialize Text", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest)));
	UTEST_OK("Deserialize Text", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Dest), FDcPropertyDatum(&Expect)));

	return true;
}

DC_TEST("DataConfig.Core.Property.TextEqualCheck")
{
	FDcTestStructWithText Foo;
	FDcTestStructWithText Bar;

	UTEST_OK("Text Equal Check", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Foo), FDcPropertyDatum(&Bar)));

	Foo.TextFieldAlpha = NSLOCTEXT("DataConfig.Tests", "FOO_ALT", "Foo Alt");
	Bar.TextFieldAlpha = NSLOCTEXT("DataConfig.Tests", "BAR_ALT", "Foo Alt");

	UTEST_DIAG("Text Equal Check", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Foo), FDcPropertyDatum(&Bar)),
		DcDReadWrite, DataTypeUnequalLhsRhs);

	Bar.TextFieldAlpha = NSLOCTEXT("DataConfig.Tests.Alt", "FOO_ALT", "Foo Alt");

	UTEST_DIAG("Text Equal Check", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Foo), FDcPropertyDatum(&Bar)),
		DcDReadWrite, DataTypeUnequalLhsRhs);

	Bar.TextFieldAlpha = FText();

	UTEST_DIAG("Text Equal Check", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Foo), FDcPropertyDatum(&Bar)),
		DcDReadWrite, DataTypeUnequalLhsRhs);

	Bar.TextFieldAlpha = NSLOCTEXT("DataConfig.Tests", "FOO_ALT", "Foo Alt");

	UTEST_OK("Text Equal Check", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Foo), FDcPropertyDatum(&Bar)));

	return true;
}

DC_TEST("DataConfig.Core.Property.SkipReadWrites")
{

	{
		FDcTestStructWithDefaults Dest;
		Dest.StrArrFieldWithDefault = {"One"};
		Dest.StrSetFieldWithDefault = {"One"};
		Dest.StringIntMapFieldWithDefault = { {"One", 1} };

		FDcPropertyReader Reader{FDcPropertyDatum(&Dest)};
		UTEST_OK("Skip Read", Reader.ReadStructRoot());
		UTEST_OK("Skip Read", Reader.ReadName(nullptr)); /// StrFieldWithDefault
		UTEST_OK("Skip Read", Reader.SkipRead()); /// StrFieldWithDefault
		UTEST_OK("Skip Read", Reader.ReadName(nullptr)); /// IntFieldWithDefault
		UTEST_OK("Skip Read", Reader.SkipRead()); /// IntFieldWithDefault

		UTEST_OK("Skip Read", Reader.ReadName(nullptr));
		UTEST_OK("Skip Read", Reader.ReadArrayRoot());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.ReadArrayEnd());

		UTEST_OK("Skip Read", Reader.ReadName(nullptr));
		UTEST_OK("Skip Read", Reader.ReadSetRoot());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.ReadSetEnd());

		UTEST_OK("Skip Read", Reader.ReadName(nullptr));
		UTEST_OK("Skip Read", Reader.ReadMapRoot());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.ReadMapEnd());
	}

	{
		FDcTestStructWithDefaults Dest;
		Dest.StrArrFieldWithDefault = {"One"};
		Dest.StrSetFieldWithDefault = {"One"};
		Dest.StringIntMapFieldWithDefault = { {"One", 1} };

		FDcPropertyWriter Writer{FDcPropertyDatum(&Dest)};

		UTEST_OK("Skip Write", Writer.WriteStructRoot());
		UTEST_OK("Skip Write", Writer.WriteName(TEXT("StrArrFieldWithDefault")));
		/// Since we changed WriteArray/Set/Map to empty the container, skip write doesn't make sense
		/// in this cases. Now it fails explicitly with SkipNotSupported.
		/// If needed we can add an override for WriteArray/Set/Map root to not empty the container.
		UTEST_OK("Skip Write", Writer.WriteArrayRoot());
		UTEST_DIAG("Skip Write", Writer.SkipWrite(),
			DcDReadWrite, SkipNotSupported);
		UTEST_OK("Skip Write", Writer.WriteArrayEnd());

		UTEST_OK("Skip Write", Writer.WriteName(TEXT("StrSetFieldWithDefault")));
		UTEST_OK("Skip Write", Writer.WriteSetRoot());
		UTEST_DIAG("Skip Write", Writer.SkipWrite(),
			DcDReadWrite, SkipNotSupported);
		UTEST_OK("Skip Write", Writer.WriteSetEnd());

		UTEST_OK("Skip Write", Writer.WriteName(TEXT("StringIntMapFieldWithDefault")));
		UTEST_OK("Skip Write", Writer.WriteMapRoot());
		UTEST_DIAG("Skip Write", Writer.SkipWrite(),
			DcDReadWrite, SkipNotSupported);
		UTEST_OK("Skip Write", Writer.WriteMapEnd());
		UTEST_OK("Skip Write", Writer.WriteStructEnd());
	}

	auto IntArrProp = DcPropertyUtils::FDcPropertyBuilder::Int().ArrayDim(3).LinkOnScope();

	{
		int SrcIntArr[3] = {1, 2, 3};

		FDcPropertyDatum DestDatum{IntArrProp.Get(), &SrcIntArr};
		FDcPropertyReader Reader(DestDatum);

		/// scalar ints don't empty arrays though
		UTEST_OK("Skip Read", Reader.ReadArrayRoot());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.SkipRead());
		UTEST_OK("Skip Read", Reader.ReadArrayEnd());
	}

	{
		int SrcIntArr[3] = {1, 2, 3};
		using namespace DcPropertyUtils;

		FDcPropertyDatum DestDatum{IntArrProp.Get(), &SrcIntArr};
		FDcPropertyWriter Writer(DestDatum);

		/// scalar ints don't empty arrays though
		UTEST_OK("Skip Write", Writer.WriteArrayRoot());
		UTEST_OK("Skip Write", Writer.SkipWrite());
		UTEST_OK("Skip Write", Writer.SkipWrite());
		UTEST_OK("Skip Write", Writer.SkipWrite());
		UTEST_OK("Skip Write", Writer.WriteArrayEnd());
	}

	return true;
}
