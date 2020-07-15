
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FDataConfigRuntimeModule
	: public IModuleInterface
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }
};


IMPLEMENT_MODULE(FDataConfigRuntimeModule, DataConfigRuntime);
