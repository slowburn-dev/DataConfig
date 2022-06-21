#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "HAL/FeedbackContextAnsi.h"
#include "HAL/ExceptionHandling.h"

struct FDcAutomationFeedbackContext : public FFeedbackContextAnsi
{
	void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		if (Category == FName(TEXT("LogDataConfigCore"))
			&& Verbosity == ELogVerbosity::Display)
		{
			LocalPrint(V);
			LocalPrint(TEXT("\n"));
		}
		else if (Category == FName(TEXT("LogDataConfigCore"))
			&& Verbosity == ELogVerbosity::NoLogging)
		{
			//	pass
		}
		else
		{
			FFeedbackContextAnsi::Serialize(V, Verbosity, Category);
		}
	}
};

FString FDcAutomationBase::CheckUniqueName(const FString& InName)
{
	check(!FAutomationTestFramework::Get().ContainsTest(InName));
	return InName;
}

uint32 FDcAutomationBase::GetTestFlags() const
{
	return FLAGS;
}

uint32 FDcAutomationBase::GetRequiredDeviceNum() const
{
	return 1;
}

bool FDcAutomationBase::TestOk(const TCHAR* Description, const FDcResult& Result)
{
	if (!Result.Ok())
	{
		AddError(FString::Printf(TEXT("%s: DcResult isn't Ok"), Description), 1);
		return false;
	}
	return true;
}

bool FDcAutomationBase::TestOk(const FString& Description, const FDcResult& Result)
{
	return TestOk(*Description, Result);
}

bool FDcAutomationBase::TestDiagnostic(const TCHAR* Description, const FDcResult& Result, uint16 Category, uint16 Code)
{
	if (Result.Ok())
	{
		AddError(FString::Printf(TEXT("%s: Expect DcResult fail but it's OK"), Description), 1);
		return false;
	}

	int DiagCount = DcEnv().Diagnostics.Num();
	if (DiagCount != 1)
	{
		AddError(FString::Printf(TEXT("%s: Expect exactly 1 diagnostic found %d"), Description, DiagCount), 1);
		return false;
	}

	FDcDiagnostic& Diag = DcEnv().GetLastDiag();
	if (Diag.Code.CategoryID != Category
		|| Diag.Code.ErrorID != Code)
	{
		AddError(FString::Printf(TEXT("%s: Last diag unmatch, Expect: (%d, %d), Actual, (%d, %d)"), 
			Description, Category, Code, Diag.Code.CategoryID, Diag.Code.ErrorID
		), 1);
		return false;
	}

	//	eat the diag on test succeed
	DcEnv().Diagnostics.Empty();

	return true;
}

bool FDcAutomationBase::TestDiagnostic(const FString& Description, const FDcResult& Result, uint16 Category, uint16 Code)
{
	return TestDiagnostic(*Description, Result, Category, Code);
}

bool FDcAutomationBase::RunTest(const FString& Parameters)
{
	bool bRet = DcRunTestBody(Parameters);
	DcEnv().FlushDiags();
	return bRet;
}

#if PLATFORM_WINDOWS && !PLATFORM_SEH_EXCEPTIONS_DISABLED
static int32 _Win32DumpStackAndExit(Windows::LPEXCEPTION_POINTERS ExceptionInfo)
{
	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)FMemory::SystemMalloc(StackTraceSize);

	StackTrace[0] = 0;
	FPlatformStackWalk::StackWalkAndDumpEx(
		StackTrace,
		StackTraceSize,
		7,
		FGenericPlatformStackWalk::EStackWalkFlags::FlagsUsedWhenHandlingEnsure);

	puts(StackTrace);
	FMemory::SystemFree(StackTrace);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

FDcAutomationConsoleRunner::FArgs FDcAutomationConsoleRunner::FromCommandlineTokens(const TArray<FString>& Tokens)
{
	FDcAutomationConsoleRunner::FArgs Args;
	if (Tokens.Num() == 0)
	{
		//	default filter
		//	ends with `.` to filter out DataConfigBenchmark
		Args.Filters.Add(TEXT("DataConfig."));	
	}

	int ParametersSplitIx = -1;
	//	note that we look for "--", but FCommandLine::Parse eats one '-'
	bool bHasParametersSplit = Tokens.Find(TEXT("-"), ParametersSplitIx);
	if (bHasParametersSplit)
	{
		for (int Ix = 0; Ix < ParametersSplitIx; Ix++)
			Args.Filters.Add(Tokens[Ix]);

		for (int Ix = ParametersSplitIx+1; Ix < Tokens.Num(); Ix++)
			Args.Parameters.Add(Tokens[Ix]);
	}
	else
	{
		for (const FString& Token : Tokens)
			Args.Filters.Add(Token);
	}

	Args.RequestedTestFilter = FDcAutomationBase::FLAGS;

	return Args;
}

void FDcAutomationConsoleRunner::Prepare(const FArgs& Args)
{
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	FAutomationTestFramework& Framework = FAutomationTestFramework::Get();

	UE_LOG(LogDataConfigCore, Display, TEXT("Filters: %s"), *FString::Join(Args.Filters, TEXT(", ")));

	Framework.SetRequestedTestFilter(Args.RequestedTestFilter);

	TArray<FAutomationTestInfo> TestInfos;
	Framework.GetValidTestNames(TestInfos);

	TestInfos.RemoveAllSwap([&Args](FAutomationTestInfo& TestInfo) {
		for (const FString& Filter : Args.Filters)
			if (!TestInfo.GetDisplayName().Contains(Filter, ESearchCase::IgnoreCase))
				return true;

		return false;
	});

	SelectedTests = MoveTemp(TestInfos);
	ParametersJoined = FString::Join(Args.Parameters, TEXT(" "));
}

namespace DcAutomationDetails
{

int32 RunTestsBody(FDcAutomationConsoleRunner* Self)
{
	FAutomationTestFramework& Framework = FAutomationTestFramework::Get();

	FDcAutomationFeedbackContext DcAutomationLog;
	TDcStoreThenReset<FFeedbackContext*> OverrideGWarn(GWarn, &DcAutomationLog);

	bool bAllSuccessful = true;

	{
		FAutomationTestFramework::Get().SetCaptureStack(true);

		int RunCount = 0;
		int SuccessCount = 0;
		int FailCount = 0;

		{
			//	shuffle tests for random execution order
			int32 LastIndex = Self->SelectedTests.Num() - 1;
			for (int32 Ix = 0; Ix < LastIndex; Ix++)
			{
				int32 SwapIx = FMath::RandRange(Ix, LastIndex);
				Self->SelectedTests.Swap(Ix, SwapIx);
			}
		}

		for (const FAutomationTestInfo& TestInfo: Self->SelectedTests)
		{
			FString TestCommand = FString::Printf(TEXT("%s %s"), *TestInfo.GetTestName(), *Self->ParametersJoined);	
			FAutomationTestExecutionInfo CurExecutionInfo;

			int32 RoleIndex = 0; 
			Framework.StartTestByName(TestCommand, RoleIndex);
			const bool CurTestSuccessful = Framework.StopTest(CurExecutionInfo);

			bAllSuccessful = bAllSuccessful && CurTestSuccessful;
			++RunCount;
			if (CurTestSuccessful) ++SuccessCount;
			else ++ FailCount;

			{
				UE_LOG(LogDataConfigCore, Display, TEXT("- %4s | %s "),
					CurTestSuccessful ? TEXT("OK") : TEXT("FAIL"),
					*TestCommand
				);

				for (const FAutomationExecutionEntry& Entry: CurExecutionInfo.GetEntries())
				{
					UE_LOG(LogDataConfigCore, Display, TEXT("* %s %s"),
						*Entry.Event.Message,
						*Entry.Event.Context
					);
					UE_LOG(LogDataConfigCore, Display, TEXT("  | %s:%d"),
						*Entry.Filename,
						Entry.LineNumber
					);
				}
			}
		}

		UE_LOG(LogDataConfigCore, Display, TEXT("Run: %4d, Success: %4d, Fail: %4d"),
			RunCount, SuccessCount, FailCount
		);
	}

	return bAllSuccessful ? 0 : -1;
}


}	// namespace DcAutomationDetails


int32 FDcAutomationConsoleRunner::RunTests()
{
	if (FPlatformMisc::IsDebuggerPresent())
	{
		return DcAutomationDetails::RunTestsBody(this);
	}
	else
	{

#if PLATFORM_WINDOWS && !PLATFORM_SEH_EXCEPTIONS_DISABLED
		__try
#endif
		{
			return DcAutomationDetails::RunTestsBody(this);
		}
#if PLATFORM_WINDOWS && !PLATFORM_SEH_EXCEPTIONS_DISABLED
		__except (_Win32DumpStackAndExit(GetExceptionInformation()))
		{
			FPlatformMisc::RequestExit(true);
			return -1;
		}
#endif
	}
}

