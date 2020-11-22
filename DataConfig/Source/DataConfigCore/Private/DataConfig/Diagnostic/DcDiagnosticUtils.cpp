#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

FDcResult DcExpect(bool CondToBeTrue) {
	if (CondToBeTrue)
		return DcOk();
	else
		return DC_FAIL(DcDCommon, PlaceHoldError);
}

