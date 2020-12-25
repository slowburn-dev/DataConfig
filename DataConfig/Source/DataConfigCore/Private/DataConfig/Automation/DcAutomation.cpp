#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/DcEnv.h"
#include "HAL/FeedbackContextAnsi.h"
#include "DataConfig/Misc/DcTemplateUtils.h"

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

int32 FDcAutomationConsoleRunner::RunTests()
{
	FAutomationTestFramework& Framework = FAutomationTestFramework::Get();

	FDcAutomationFeedbackContext DcAutomationLog;
	TDcStoreThenReset<FFeedbackContext*> OverrideGWarn(GWarn, &DcAutomationLog);

	//	TODO seh and things
	//	TODO dump in place

	//	in console the stack line capture is always in `AutomationTest.h` so disable it for now
	FAutomationTestFramework::Get().SetCaptureStack(false);

	bool bAllSuccessful = true;

	int runCount = 0;
	int successCount = 0;
	int failCount = 0;

	{
		//	shuffle tests for random execution order
		int32 LastIndex = SelectedTests.Num() - 1;
		for (int32 Ix = 0; Ix < LastIndex; Ix++)
		{
			int32 SwapIx = FMath::RandRange(Ix, LastIndex);
			SelectedTests.Swap(Ix, SwapIx);
		}
	}

	for (const FAutomationTestInfo& TestInfo: SelectedTests)
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
			UE_LOG(LogDataConfigCore, Display, TEXT("- %-32s : %-4s"),
				*TestCommand,
				CurTestSuccessful ? TEXT("OK") : TEXT("FAIL")
			);

			for (const FAutomationExecutionEntry& Entry: CurExecutionInfo.GetEntries())
			{
				UE_LOG(LogDataConfigCore, Display, TEXT("* %s %s"),
					*Entry.Event.Message,
					*Entry.Event.Context
				);
			}
		}

		if (DcEnv().Diagnostics.Num())
		{
			UE_LOG(LogDataConfigCore, Display, TEXT("-----------------------------------"));
			DcEnv().FlushDiags();
			UE_LOG(LogDataConfigCore, Display, TEXT("-----------------------------------"));
		}
	}

	UE_LOG(LogDataConfigCore, Display, TEXT("Run: %4d, Success: %4d, Fail: %4d"),
		runCount, successCount, failCount
	);

	return bAllSuccessful ? 0 : -1;
}

