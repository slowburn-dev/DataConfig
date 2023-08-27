#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticJSON.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
	UTEST_OK("Read simple relaxed JSON TCHAR string", DcNoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);

	//	try reuse
	UTEST_OK("Read simple relaxed JSON TCHAR string", Reader.SetNewString(*Str));
	UTEST_OK("Read simple relaxed JSON TCHAR string", DcNoopPipeVisit(&Reader));
	UTEST_EQUAL("Read simple relaxed JSON TCHAR string", Reader.State, FDcJsonReader::EState::FinishedStr);


	FTCHARToUTF8 AnsiStr(*Str);
	FDcAnsiJsonReader Reader2;

	UTEST_OK("Read simple relaxed JSON ANSICHAR string", Reader2.SetNewString(AnsiStr.Get()));
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", DcNoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON ANSICHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	//	try reuse
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", Reader2.SetNewString(AnsiStr.Get()));
	UTEST_OK("Read simple relaxed JSON ANSICHAR string", DcNoopPipeVisit(&Reader2));
	UTEST_EQUAL("Read simple relaxed JSON ANSICHAR string", Reader2.State, FDcAnsiJsonReader::EState::FinishedStr);

	return true;
};


DC_TEST("DataConfig.Core.JSON.Diags")
{
	{
		FString Str = TEXT(R"(

			{
				"SameKey" : "Is",
				"SameKey" : "Not Allowed.",
			} 

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'DuplicateKey' err", DcNoopPipeVisit(&Reader), DcDJSON, DuplicatedKey);
	}

	{
		FString Str = TEXT(R"(

			{
				2 : "Two",
			} 

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'KeyMustBeString' err", DcNoopPipeVisit(&Reader), DcDJSON, KeyMustBeString);
	}

	{
		FString Str = TEXT(R"(

			[1 2 3 4 5]

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'ExpectComma' err", DcNoopPipeVisit(&Reader), DcDJSON, ExpectComma);
	}


	{
		FString Str = TEXT(R"(

			{
				"One": 1
				"Two": 1
			}

		)");
		FDcJsonReader Reader(Str);

		UTEST_DIAG("Expect 'DuplicateKey' err", DcNoopPipeVisit(&Reader), DcDJSON, ExpectComma);
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
		//	no escape char
		const char* UTF8Literal =  "\"\xe4\xbd\xa0\xe5\xa5\xbd\"";

		FDcAnsiJsonReader Reader(UTF8Literal);
		FString LoadedStr;
		UTEST_OK("Read UTF8 String", Reader.ReadString(&LoadedStr));
		UTEST_EQUAL("Read UTF8 String", LoadedStr, TEXT("\u4f60\u597d"));
	}

	{
		//	has escape char
		const char* UTF8Literal =  "\"\\t\xe4\xbd\xa0\xe5\xa5\xbd\"";

		FDcAnsiJsonReader Reader(UTF8Literal);
		FString LoadedStr;
		UTEST_OK("Read UTF8 String", Reader.ReadString(&LoadedStr));
		UTEST_EQUAL("Read UTF8 String", LoadedStr, TEXT("\t\u4f60\u597d"));
	}

	return true;
}

DC_TEST("DataConfig.Core.JSON.TCHARUnicode")
{
	{
		//	no escape char
		const TCHAR* TCharLiteral = TEXT("\"\u4f60\u597d\"");

		FDcJsonReader Reader(TCharLiteral);
		FString LoadedStr;
		UTEST_OK("Read TCHARUnicode String", Reader.ReadString(&LoadedStr));
		UTEST_EQUAL("Read TCHARUnicode String", LoadedStr, TEXT("\u4f60\u597d"));
	}

	{
		//	has escape char
		const char* UTF8Literal =  "\"\\t\u4f60\u597d\"";

		FDcAnsiJsonReader Reader(UTF8Literal);
		FString LoadedStr;
		UTEST_OK("Read TCHARUnicode String", Reader.ReadString(&LoadedStr));
		UTEST_EQUAL("Read TCHARUnicode String", LoadedStr, TEXT("\t\u4f60\u597d"));
	}

	return true;
}

namespace DcTestJsonDetails
{

struct FDcJsonTestFixure
{
	FString Filename;
	FString Content;
};

static FDcResult RunSingleJsonFixture(FDcJsonTestFixure& Fixture)
{
	enum class EAccept
	{
		Unknown,
		Yes,
		No,
		Irrelevant,
	};

	FString Basename = FPaths::GetBaseFilename(Fixture.Filename);
	EAccept Accept = EAccept::Unknown;
	if (Basename.StartsWith(TEXT("y_")))
		Accept = EAccept::Yes;
	else if (Basename.StartsWith(TEXT("n_")))
		Accept = EAccept::No;
	else if (Basename.StartsWith(TEXT("i_")))
		Accept = EAccept::Irrelevant;
	
	FDcJsonReader Reader(Fixture.Content);
	DcEnv().FlushDiags();

	if (Accept == EAccept::Irrelevant)
	{
		TDcStoreThenReset<bool> ScopedExpectFail(DcEnv().bExpectFail, true);

		FDcResult Ret = DcNoopPipeVisit(&Reader);
		Ret.Ok();	//	discard the result
		
		DcEnv().Diagnostics.Empty();
	}
	else if (Accept == EAccept::Yes)
	{
		FDcResult Ret = DcNoopPipeVisit(&Reader);
		if (!Ret.Ok())
		{
			return DC_FAIL(DcDCommon, CustomMessage)
				<< FString::Printf(TEXT("DcJsonFixture: Expect Accept but failed: %s"), *Basename);
		}
	}
	else if (Accept == EAccept::No)
	{
		TDcStoreThenReset<bool> ScopedExpectFail(DcEnv().bExpectFail, true);

		FDcResult Ret = DcNoopPipeVisit(&Reader);
		FDcResult Finish = Reader.FinishRead();
		if (Ret.Ok() && Finish.Ok())
		{
			return DC_FAIL(DcDCommon, CustomMessage)
				<< FString::Printf(TEXT("DcJsonFixture: Expect Reject but succeeded: %s"), *Basename);
		}
		else
		{
			DcEnv().Diagnostics.Empty();
		}
	}
	else
	{
		return DC_FAIL(DcDCommon, CustomMessage)
			<< FString::Printf(TEXT("DcTests: Unknown Acceptance: %s"), *Basename);
	}

	return DcOk();
}

} // namespace DcTestJsonDetails

DC_TEST("DataConfig.Core.JSON.DcJSONFixtures")
{
	using namespace DcTestJsonDetails;

	IFileManager& FileManager = IFileManager::Get();
	TArray<FDcJsonTestFixure> Fixtures;
	
	FileManager.IterateDirectory(*DcGetFixturePath(TEXT("DcJSONFixtures")), [&](const TCHAR* VisitFilename, bool VisitIsDir)
	{
		FString Filename(VisitFilename);
		if (Filename.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
		{
			FString JsonStr;
			verify(FFileHelper::LoadFileToString(JsonStr, VisitFilename));
			Fixtures.Emplace(FDcJsonTestFixure{VisitFilename, MoveTemp(JsonStr)});
		}
		
		return true;
	});

	bool bAllPass = true;
	for (FDcJsonTestFixure& Fixture : Fixtures)
	{
		if (!RunSingleJsonFixture(Fixture).Ok())
			bAllPass =false;
	}
	
	return bAllPass;
}


DC_TEST("DataConfig.Core.JSON.JSONTestSuiteParsing")
{
	using namespace DcTestJsonDetails;

	IFileManager& FileManager = IFileManager::Get();
	TArray<FDcJsonTestFixure> Fixtures;
	
	FileManager.IterateDirectory(*DcGetFixturePath(TEXT("JSONTestSuiteParsing")), [&](const TCHAR* VisitFilename, bool VisitIsDir)
	{
		FString Filename(VisitFilename);
		if (!Parameters.IsEmpty())
		{
			if (!Filename.Contains(Parameters))
				return true;
		}
		
		if (Filename.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
		{
			FString JsonStr;
			verify(FFileHelper::LoadFileToString(JsonStr, VisitFilename));
			Fixtures.Emplace(FDcJsonTestFixure{VisitFilename, MoveTemp(JsonStr)});
		}
		
		return true;
	});

	bool bAllPass = true;
	for (FDcJsonTestFixure& Fixture : Fixtures)
	{
		if (!RunSingleJsonFixture(Fixture).Ok())
			bAllPass =false;
	}
	
	return bAllPass;
}

