#include "Old/Adhocs.h"
#include "UObject/UnrealType.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Source/DcSourceTypes.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "Misc/CString.h"
#include "Misc/FileHelper.h"
#include "ProfilingDebugging/ScopedTimers.h"

void JsonReader1()
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"Hello" : "JSON the shit",
			"Truthy" : true,
			"Btoy" : -2538.1e-3,
			"Falsy" : false,
			"Nest1" : {
				"Nest1" : "yeah",
			},
			// /*  /* helllllo ??? */
			"Nest2" : {
				"Nest2" : "wow",
				"Inty" : "dow",
			},
			"Arr" : [
				"these", "are", "my", "twisted",
			],
			"Inty" : null,
		}

	)");
	Reader.SetNewString(*Str);

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FDcPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
		FDcPipeVisitor PrettyPrintVisit(&Reader, &Writer);
		FDcResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}
}

static_assert(TIsSame<TCHAR, WIDECHAR>::Value , "TCHAR is WIDECHAR on pc atleast");

void SourceTypes()
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);

	static char* _text = "these are my twisted words";
	FString WhatText(5, _text + 5);

	static TCHAR* _tchar_text = L"also my twisted words";
	FString TCharText(5, _tchar_text + 5);

	FDcAnsiSourceBuffer Buf(_text);
	FDcAnsiSourceRef Ref{ &Buf, 6, 5 };

	FDcSourceBuffer TBuf(_tchar_text);
	FDcSourceRef TRef{ &TBuf, 0, 4 };

	UE_LOG(LogDataConfigCore, Display, TEXT("'%s' '%s' '%s' '%s'"), *WhatText, *TCharText, *Ref.ToString(), *TRef.ToString());

}

void JsonFail1()
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(
	{
		fuck
	}
	)");
	Reader.SetNewString(*Str);

	Reader.ReadNil();

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FDcPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
		FDcPipeVisitor PrettyPrintVisit(&Reader, &Writer);
		FDcResult Ret = PrettyPrintVisit.PipeVisit();
		if (!Ret.Ok())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
		}
	}

}

void JsonCanada()
{
	FDcAnsiJsonReader Reader;

	TArray<uint8> Result;
	FFileHelper::LoadFileToArray(Result, TEXT(R"(C:\DevUE\projects\DataConfig\Reference\simdjson-0.6.0\jsonexamples\canada.json)"));

	Reader.SetNewString(Result);

	FString Str;
	//Reader.SetNewString(Str.GetCharArray());

	{
		FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);

		double ExecTime = 0;
		{
			FScopedDurationTimer Timer(ExecTime);
			//FDcPrettyPrintWriter Writer(*(FOutputDevice*)GWarn);
			FDcNoopWriter Writer;
			FDcPipeVisitor PrettyPrintVisit(&Reader, &Writer);
			FDcResult Ret = PrettyPrintVisit.PipeVisit();
			if (!Ret.Ok())
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit failed --"));
			}
		}

		UE_LOG(LogDataConfigCore, Display, TEXT("- pipe visit time: %.2f --"), ExecTime);
	}

}
