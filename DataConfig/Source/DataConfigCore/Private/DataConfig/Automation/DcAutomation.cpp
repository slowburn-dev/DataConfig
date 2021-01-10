#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/DcEnv.h"
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
		else
		{
			FFeedbackContextAnsi::Serialize(V, Verbosity, Category);
		}
	}
};

uint32 FDcAutomationBase::GetTestFlags() const
{
	return _FLAGS;
}

uint32 FDcAutomationBase::GetRequiredDeviceNum() const
{
	return 1;
}

bool FDcAutomationBase::TestOk(const TCHAR* Description, const FDcResult& Result)
{
	if (!Result.Ok())
	{
		AddError(FString::Printf(TEXT("%s: DcResult isn't Ok"), Description));
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
		AddError(FString::Printf(TEXT("%s: Expect DcResult fail but it's OK"), Description));
		return false;
	}

	int DiagCount = DcEnv().Diagnostics.Num();
	if (DiagCount != 1)
	{
		AddError(FString::Printf(TEXT("%s: Expect exactly 1 diagnostic found %d"), Description, DiagCount));
		return false;
	}

	FDcDiagnostic& Diag = DcEnv().GetLastDiag();
	if (Diag.Code.CategoryID != Category
		|| Diag.Code.ErrorID != Code)
	{
		AddError(FString::Printf(TEXT("%s: Last diag unmatch, Expect: (%d, %d), Actual, (%d, %d)"), 
			Description, Category, Code, Diag.Code.CategoryID, Diag.Code.ErrorID
		));
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



void FDcAutomationConsoleRunner::Prepare(const FArgs& Args)
{
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	FAutomationTestFramework& Framework = FAutomationTestFramework::Get();

	UE_LOG(LogDataConfigCore, Display, TEXT("Filters: %s"), *FString::Join(Args.Filters, TEXT(", ")));

	Framework.SetRequestedTestFilter(Args.RequestedTestFilter);

	TArray<FAutomationTestInfo> TestInfos;
	Framework.GetValidTestNames(TestInfos);

	TestInfos.RemoveAll([&Args](FAutomationTestInfo& TestInfo) {
		for (const FString& Filter : Args.Filters)
			if (!TestInfo.GetDisplayName().Contains(Filter, ESearchCase::IgnoreCase))
				return true;

		return false;
	});

	SelectedTests = MoveTemp(TestInfos);
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
		//	in console the stack line capture is always in `AutomationTest.h` so disable it for now
		FAutomationTestFramework::Get().SetCaptureStack(false);


		int runCount = 0;
		int successCount = 0;
		int failCount = 0;

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
			FString TestCommand = TestInfo.GetTestName();
			FAutomationTestExecutionInfo CurExecutionInfo;

			int32 RoleIndex = 0; 
			Framework.StartTestByName(TestCommand, RoleIndex);
			const bool CurTestSuccessful = Framework.StopTest(CurExecutionInfo);

			bAllSuccessful = bAllSuccessful && CurTestSuccessful;
			++runCount;
			if (CurTestSuccessful) ++successCount;
			else ++ failCount;

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
				}
			}
		}

		UE_LOG(LogDataConfigCore, Display, TEXT("Run: %4d, Success: %4d, Fail: %4d"),
			runCount, successCount, failCount
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

	//	TODO linux equivelent
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

