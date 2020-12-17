#include "DataConfig/DcEnv.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticAll.inl"
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

namespace DcEnvDetails{

bool bInitialized = false;

}

void DcStartUp(EDcInitializeAction InAction)
{
	DiagGroups.Emplace(&DCommonDetails);
	DiagGroups.Emplace(&DPropertyReadWriteDetails);
	DiagGroups.Emplace(&DJSONDetails);
	DiagGroups.Emplace(&DDeserializeDetails);

	DcPushEnv();
	DcEnvDetails::bInitialized = true;

	if (InAction == EDcInitializeAction::SetAsConsole)
	{
		DcEnv().DiagConsumer = MakeShareable(new FDcDefaultLogDiagnosticConsumer());
	}
}

void DcShutDown()
{
	DcPopEnv();
	DiagGroups.RemoveAt(0, DiagGroups.Num());

	DcEnvDetails::bInitialized = false;
}

bool DcIsInitialized()
{
	return DcEnvDetails::bInitialized;
}

