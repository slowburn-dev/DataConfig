#include "DataConfig/EditorExtra/Tests/DcCoreTestsCommandlet.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Automation/DcAutomation.h"

UDcCoreTestsCommandlet::UDcCoreTestsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UDcCoreTestsCommandlet::Main(const FString& Params)
{
	UE_SET_LOG_VERBOSITY(LogDataConfigCore, Display);

	UE_LOG(LogDataConfigCore, Display, TEXT("================================================================="));

	TArray<FString> Tokens;
	TArray<FString> Switches;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches);
	Tokens.RemoveAt(0); // 0 is always commandlet name

	FDcAutomationConsoleRunner Runner;
	Runner.Prepare(FDcAutomationConsoleRunner::FromCommandlineTokens(Tokens));
	int32 Ret = Runner.RunTests();

	UE_LOG(LogDataConfigCore, Display, TEXT("================================================================="));

	return Ret;

}
