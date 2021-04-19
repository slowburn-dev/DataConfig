#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Automation/DcAutomation.h"

static FDcResult _NoopPipeVisit(FDcReader* Reader)
{
	FDcNoopWriter Writer{};
	FDcPipeVisitor PipeVisitor(Reader, &Writer);
	return PipeVisitor.PipeVisit();
}

DC_TEST("DataConfig.Core.JSON.Reader1")
{
	FDcJsonReader Reader;

	FString Str = TEXT(R"(

		{
			"Hello" : "JSON",
			"Truthy" : true,
			"Btoy" : -2538.1e-3,
			"Falsy" : false,
			"Nest1" : {
				"Nest1" : "yeah",
			},
			// /*  /* helllllo ??? */
			"Nest2" : {
				"Nest2" : "woot",
				"Inty" : "doot",
			},
			"Arr" : [
				"these", "are", "my", "twisted",
			],
			"Inty" : null,	/* allow trailing comma */
		} 

	)");

	UTEST_OK("Read simple relaxed JSON TCHAR string", Reader.SetNewString(*Str));
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);

	//	try reuse
	UTEST_OK("Read simple relaxed JSON TCHAR string", Reader.SetNewString(*Str));
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);


	FTCHARToUTF8 AnsiStr(*Str);
	FDcAnsiJsonReader Reader2;

	UTEST_OK("Read simple relaxed JSON ANSICHAR string", Reader2.SetNewString(AnsiStr.Get()));
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", _NoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON ANSICHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	//	try reuse
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", Reader2.SetNewString(AnsiStr.Get()));
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", _NoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON ANSICHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	return true;
};


DC_TEST("DataConfig.Core.JSON.ReaderErrors")
{
	{
		FString Str = TEXT(R"(

			{
				"SameKey" : "Is",
				"SameKey" : "Not Allowed.",
			} 

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'DuplicateKey' err", _NoopPipeVisit(&Reader), DcDJSON, DuplicatedKey);
	}

	{
		FString Str = TEXT(R"(

			{
				2 : "Two",
			} 

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'KeyMustBeString' err", _NoopPipeVisit(&Reader), DcDJSON, KeyMustBeString);
	}

	{
		FString Str = TEXT(R"(

			[1 2 3 4 5]

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'ExpectComma' err", _NoopPipeVisit(&Reader), DcDJSON, ExpectComma);
	}


	{
		FString Str = TEXT(R"(

			{
				"One": 1
				"Two": 1
			}

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'DuplicateKey' err", _NoopPipeVisit(&Reader), DcDJSON, ExpectComma);
	}


	return true;
}

DC_TEST("DataConfig.Core.JSON.EndRead")
{
	{
		FString Str = TEXT(R"(

			{}
			"Trailing"

		)");
		FDcJsonReader Reader(Str);

		UTEST_OK("Read json", Reader.ReadMapRoot());
		UTEST_OK("Read json", Reader.ReadMapEnd());
		UTEST_DIAG("Expect 'TrailingToken' err", Reader.FinishRead(), DcDJSON, UnexpectedTrailingToken);
	}

	{
		FString Str = TEXT(R"(

			{}
			//	trailing comments are
			/*	just fine */
		)");
		FDcJsonReader Reader(Str);

		UTEST_OK("Read json", Reader.ReadMapRoot());
		UTEST_OK("Read json", Reader.ReadMapEnd());
		UTEST_OK("Expect end ok", Reader.FinishRead());
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.UTF8")
{
	{
		const char* UTF8Literal =  "\"\xe4\xbd\xa0\xe5\xa5\xbd\"";

		FDcAnsiJsonReader Reader(UTF8Literal);
		FString LoadedStr;
		UTEST_OK("Read UTF8 String", Reader.ReadString(&LoadedStr));

		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		UTEST_EQUAL("Read UTF8 String", LoadedStr, TEXT("\u4f60\u597d"));
	}

	return true;
}




