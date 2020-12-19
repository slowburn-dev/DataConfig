#include "DataConfig/Automation/DcAutomation.h"

uint32 FDcAutomationBase::GetTestFlags() const
{
	return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter;
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

void FDcAutomationConsoleRunner::Prepare()
{


}

void FDcAutomationConsoleRunner::RunTests()
{
}

