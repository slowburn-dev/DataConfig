#include "DataConfig/DcEnv.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "Containers/BasicArray.h"

TBasicArray<FDcEnv> gDcEnvs;

FDcEnv& DcEnv()
{
	check(DcIsInitialized());
	return gDcEnvs[gDcEnvs.Num() - 1];
}

FDcEnv& DcPushEnv()
{
	return gDcEnvs[gDcEnvs.Emplace()];
}

void DcPopEnv()
{
	gDcEnvs.RemoveAt(gDcEnvs.Num() - 1);
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

namespace DcEnvDetails {

bool bInitialized = false;

} // namespace DcEnvDetails

FDcResult DcFail()
{
	//	attach a stack trace as otherwise it's very diffcult to find
	return DC_FAIL(DcDCommon, Unhandled)
		<< FDcDiagnosticStringNoEscape(DcDiagnosticUtils::StackWalkToString(0));
}

namespace DcDCommon { extern FDcDiagnosticGroup Details; }
namespace DcDReadWrite { extern FDcDiagnosticGroup Details; }
namespace DcDJSON { extern FDcDiagnosticGroup Details; }
namespace DcDDeserialize { extern FDcDiagnosticGroup Details; }

void DcStartUp(EDcInitializeAction InAction)
{
	DcDiagGroups.Emplace(&DcDCommon::Details);
	DcDiagGroups.Emplace(&DcDReadWrite::Details);
	DcDiagGroups.Emplace(&DcDJSON::Details);
	DcDiagGroups.Emplace(&DcDDeserialize::Details);

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
	DcDiagGroups.RemoveAt(0, DcDiagGroups.Num());

	DcEnvDetails::bInitialized = false;
}

bool DcIsInitialized()
{
	return DcEnvDetails::bInitialized;
}

