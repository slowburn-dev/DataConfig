#include "DataConfig/DcEnv.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "Containers/BasicArray.h"

namespace DataConfig
{

TBasicArray<FDcEnv> Envs;

FDcEnv& DataConfig::DcEnv()
{
	check(DcIsInitialized());
	return Envs[Envs.Num() - 1];
}

FDcEnv& DataConfig::DcPushEnv()
{
	return Envs[Envs.Emplace()];
}

void DataConfig::DcPopEnv()
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


FDcResult DcExpect(bool CondToBeTrue) {
	if (CondToBeTrue)
		return DcOk();
	else
		return DcFail(DC_DIAG(DCommon, PlaceHoldError));
}

} // namespace DataConfig

