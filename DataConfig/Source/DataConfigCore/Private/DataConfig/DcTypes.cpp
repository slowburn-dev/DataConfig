#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"

DEFINE_LOG_CATEGORY(LogDataConfigCore);

bool bInitialized = false;

void DcStartUp(EDcInitializeAction InAction)
{
	bInitialized = true;
	DcPushEnv();

	if (InAction == EDcInitializeAction::SetAsConsole)
	{
		DcEnv().DiagConsumer = MakeShareable(new FDcDefaultLogDiagnosticConsumer());
	}
}

void DcShutDown()
{
	DcPopEnv();
	bInitialized = false;
}

DATACONFIGCORE_API bool DcIsInitialized()
{
	return bInitialized;
}

