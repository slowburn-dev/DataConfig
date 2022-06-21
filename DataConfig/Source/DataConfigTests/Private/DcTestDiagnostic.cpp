#include "DataConfig/DcTypes.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Source/DcHighlightFormatter.h"


namespace DcTestDiagnosticDetails
{

static FDcResult CheckDiagnosticHighlight(FAutomationTestBase* Self, FString From, FString To, int Line, int Offset, int Len)
{
	using FSourceView = TDcSourceView<TCHAR>;
	using FHighligher = THightlightFormatter<TCHAR>;
	FHighligher Highlighter;

	FSourceView View{GetData(From), From.Len()};
	FHighligher::SourceRef Ref{&View, Offset, Len};

	FString Highlight = Highlighter.FormatHighlight(Ref, Line);
	Highlight.AppendChar(TCHAR('\n'));

	if (!Self->TestEqual("Highlight", Highlight, To))
		return DC_FAIL(DcDCommon, CustomMessage) << TEXT("Diagnostic Highlight Fail");

	return DcOk();
}

} // namespace DcTestDiagnosticDetails

DC_TEST("DataConfig.Core.Diagnostic.Highlight")
{
	using namespace DcTestDiagnosticDetails;
	FString Prefix = TEXT("   "); // 3 white space prefix

	UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		these are my
		twisted
		words.

		)")),
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		1 |these are my
		2 |twisted
		  |^^^^^^^
		3 |words.

		)"), &Prefix),
		2,		// Line
		13, 7	// Offset, Num
	));

	UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		these are my
		twisted
		words.

		)")),
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		1 |these are my
		2 |twisted
		  |^^^^^^^
		3 |words.

		)"), &Prefix),
		INDEX_NONE,	// auto line number
		13, 7		// Offset, Num
	));

	UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		line1
		line2

		)")),
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		1 |line1
		2 |line2
		  |     ^

		)"), &Prefix),
		2,		// Line
		11, 1	// Offset, Num
	));

	UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		line1
		line2

		)")),
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		1 |line1
		  |     ^
		2 |line2

		)"), &Prefix),
		INDEX_NONE,	// auto line number
		5, 1		// Offset, Num
	));


	UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
		this,
		FString::ChrN(1024, TCHAR('X')),
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		1 |...XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		  |                                                                                                                                              ^

		)"), &Prefix),
		INDEX_NONE,	// auto line number
		1023, 1		// Offset, Num
	));

	{
		auto _Repeat = [](const FString Str, int N)
		{
			FString Ret;
			for (int Ix = 0; Ix < N; Ix++)
				Ret += Str;
			return Ret;
		};

		FString Source = _Repeat(TEXT("1234567890ABCD"), 15)
			+ TEXT("\n") + _Repeat(TEXT("1234567890ABCD"), 13)
			+ TEXT("\n") + _Repeat(TEXT("1234567890ABCD"), 11);

		UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
			this,
			Source,
			DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		   1 |1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD...
		   2 |1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD...
			 |^
		   3 |1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD...

			)"), &Prefix),
			INDEX_NONE,	// auto line number
			14*(15) + 1, 1		// Offset, Num
		));

		UTEST_OK("Diagnostic Highlight Format", CheckDiagnosticHighlight(
			this,
			Source,
			DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		   1 |...234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD
			 |                                                                                                                                             ^
		   2 |...234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD
		   3 |...234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD1234567890ABCD

			)"), &Prefix),
			INDEX_NONE,	// auto line number
			14*(15)-1, 1		// Offset, Num
		));

	}
		
	return true;
};

