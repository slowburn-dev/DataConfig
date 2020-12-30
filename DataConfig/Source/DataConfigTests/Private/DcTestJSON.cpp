#include "DcTestJson.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"

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

	Reader.SetNewString(*Str);
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);

	//	try reuse
	Reader.SetNewString(*Str);
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);


	FTCHARToUTF8 AnsiStr(*Str);
	FDcAnsiJsonReader Reader2;

	Reader2.SetNewString(AnsiStr.Get());
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	//	try reuse
	Reader2.SetNewString(AnsiStr.Get());
	UTEST_OK("Read simple relaxed JSON TCHAR string", _NoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	return true;
};


DC_TEST("DataConfig.Core.JSON.ReaderErrors")
{
	{
		FDcJsonReader Reader;

		FString Str = TEXT(R"(

			{
				"SameKey" : "Is",
				"SameKey" : "Not Allowed.",
			} 

		)");

		Reader.SetNewString(*Str);

		UTEST_DIAG("Expect 'DuplicateKey' err", _NoopPipeVisit(&Reader), DcDJSON, DuplicatedKey);
	}

	{
		FDcJsonReader Reader;

		FString Str = TEXT(R"(

			{
				2 : "Two",
			} 

		)");

		Reader.SetNewString(*Str);

		UTEST_DIAG("Expect 'KeyMustBeString' err", _NoopPipeVisit(&Reader), DcDJSON, KeyMustBeString);
	}

	{
		FDcJsonReader Reader;

		FString Str = TEXT(R"(

			[1 2 3 4 5]

		)");

		Reader.SetNewString(*Str);

		UTEST_DIAG("Expect 'ExpectComma' err", _NoopPipeVisit(&Reader), DcDJSON, ExpectComma);
	}


	{
		FDcJsonReader Reader;

		FString Str = TEXT(R"(

			{
				"One": 1
				"Two": 1
			}

		)");

		Reader.SetNewString(*Str);

		UTEST_DIAG("Expect 'ExpectComma' err", _NoopPipeVisit(&Reader), DcDJSON, ExpectComma);
	}

	return true;
}


