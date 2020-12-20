#include "DataConfig/Automation/DcAutomation.h"

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
	FAutomationTestFramework& Framework = FAutomationTestFramework::Get();

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

	//	TODO seh and things
	//	TODO dump in place
	TMap<FString, FAutomationTestExecutionInfo> ExecutionInfoMap;
	FAutomationTestFramework::Get().SetCaptureStack(true);

	bool bAllSuccessful = true;

	for (const FAutomationTestInfo& TestInfo: SelectedTests)
	{
		FString TestCommand = TestInfo.GetTestName();
		FAutomationTestExecutionInfo& CurExecutionInfo = ExecutionInfoMap.Add(TestCommand, FAutomationTestExecutionInfo());

		int32 RoleIndex = 0;  //always default to "local" role index.  Only used for multi-participant tests
		Framework.StartTestByName(TestCommand, RoleIndex);
		const bool CurTestSuccessful = Framework.StopTest(CurExecutionInfo);

		bAllSuccessful = bAllSuccessful && CurTestSuccessful;
	}

	return bAllSuccessful ? 0 : -1;
}

