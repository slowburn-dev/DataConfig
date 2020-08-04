#include "DataConfig/DcEnv.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "Containers/BasicArray.h"

namespace DataConfig
{

TBasicArray<FEnv> Envs;

FEnv& DataConfig::Env()
{
	check(IsInitialized());
	return Envs[Envs.Num() - 1];
}

FEnv& DataConfig::PushEnv()
{
	return Envs[Envs.Emplace()];
}

void DataConfig::PopEnv()
{
	Envs.RemoveAt(Envs.Num() - 1);
}

FScopedEnv::FScopedEnv()
{
	PushEnv();
}

FScopedEnv::~FScopedEnv()
{
	PopEnv();
}


FDiagnostic& FEnv::Diag(FErrorCode InErr)
{
	return Diagnostics[Diagnostics.Emplace(InErr)];
}

void FEnv::FlushDiags()
{
	if (DiagConsumer.IsValid())
	{
		for (FDiagnostic& Diag : Diagnostics)
			DiagConsumer->HandleDiagnostic(Diag);
	}

	Diagnostics.Empty();
}

FEnv::~FEnv()
{
	FlushDiags();
}


FResult Expect(bool CondToBeTrue) {
	if (CondToBeTrue)
		return Ok();
	else
		return Fail(DIAG(DCommon, PlaceHoldError));
}

} // namespace DataConfig

