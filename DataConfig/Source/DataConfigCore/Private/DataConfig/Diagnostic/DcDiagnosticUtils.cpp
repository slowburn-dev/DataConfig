#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

#include "DataConfig/Reader/DcReader.h"

FDcResult DcExpect(bool CondToBeTrue) {
	if (CondToBeTrue)
		return DcOk();
	else
		return DC_FAIL(DcDCommon, PlaceHoldError);
}

FDcResult DcReadNextExpect(FDcReader& Reader, EDcDataEntry Expect)
{
	EDcDataEntry Actual;
	DC_TRY(Reader.ReadNext(&Actual));
	return DcExpect(Actual == Expect, [&] {
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< Expect << Actual
			<< Reader;
	});
}

