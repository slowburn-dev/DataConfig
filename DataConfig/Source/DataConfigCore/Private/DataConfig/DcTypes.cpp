#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"

DEFINE_LOG_CATEGORY(LogDataConfigCore);

namespace DataConfig {

bool bInitialized = false;

void StartUp(EInitializeAction InAction)
{
	bInitialized = true;
	PushEnv();

	if (InAction == EInitializeAction::SetAsConsole)
	{
		Env().DiagConsumer = MakeShareable(new FDefaultLogDiagnosticConsumer());
	}
}

void ShutDown()
{
	PopEnv();
	bInitialized = false;
}

DATACONFIGCORE_API bool IsInitialized()
{
	return bInitialized;
}

} // namespace DataConfig
