#include "DataConfig/DcEnv.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "Containers/BasicArray.h"

TBasicArray<FDcEnv> Envs;

FDcEnv& DcEnv()
{
	check(DcIsInitialized());
	return Envs[Envs.Num() - 1];
}

FDcEnv& DcPushEnv()
{
	return Envs[Envs.Emplace()];
}

void DcPopEnv()
{
	Envs.RemoveAt(Envs.Num() - 1);
}

FDcScopedEnv::FDcScopedEnv()
{
	DcPushEnv();
}

FDcScopedEnv::~FDcScopedEnv()
{
	DcPopEnv();
}


FDcDiagnostic& FDcEnv::Diag(FDcErrorCode InErr)
{
	return Diagnostics[Diagnostics.Emplace(InErr)];
}

void FDcEnv::FlushDiags()
{
	if (DiagConsumer.IsValid())
	{
		for (FDcDiagnostic& Diag : Diagnostics)
			DiagConsumer->HandleDiagnostic(Diag);
	}

	Diagnostics.Empty();
}

FDcEnv::~FDcEnv()
{
	FlushDiags();
}

namespace DcEnv_Private{

bool bInitialized = false;

}

void DcStartUp(EDcInitializeAction InAction)
{
	DcEnv_Private::bInitialized = true;
	DcPushEnv();

	if (InAction == EDcInitializeAction::SetAsConsole)
	{
		DcEnv().DiagConsumer = MakeShareable(new FDcDefaultLogDiagnosticConsumer());
	}
}

void DcShutDown()
{
	DcPopEnv();
	DcEnv_Private::bInitialized = false;
}

bool DcIsInitialized()
{
	return DcEnv_Private::bInitialized;
}

