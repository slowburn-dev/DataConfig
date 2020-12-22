#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"


FDcResult DcExpect(bool CondToBeTrue) {
	if (CondToBeTrue)
		return DcOk();
	else
		return DC_FAIL(DcDCommon, PlaceHoldError);
}

FDcResult DcReadNextExpect(FDcReader& Reader, EDcDataEntry Expect)
{
	EDcDataEntry Actual;
	DC_TRY(Reader.PeekRead(&Actual));
	return DcExpect(Actual == Expect, [&] {
		return DC_FAIL(DcDReadWrite, DataTypeMismatch)
			<< Expect << Actual
			<< Reader;
	});
}

namespace DcDiagnosticUtils
{

void DcDiagnosticUtils::AmendDiagnostic(FDcDiagnostic& Diag, FDcReader* Reader, FDcWriter* Writer)
{
	bool bHasReaderDiag = false;
	bool bHasWriterDiag = false;
	for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
	{
		if (Highlight.Owner == Reader)
			bHasReaderDiag = true;
		else if (Highlight.Owner == Writer)
			bHasWriterDiag = true;
	}

	if (!bHasReaderDiag)
		Reader->FormatDiagnostic(Diag);
	if (!bHasWriterDiag)
		Writer->FormatDiagnostic(Diag);
}

FString StackWalkToString(int32 IgnoreCount)
{
	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)FMemory::SystemMalloc(StackTraceSize);

	StackTrace[0] = 0;
	FPlatformStackWalk::StackWalkAndDumpEx(
		StackTrace,
		StackTraceSize,
		IgnoreCount,
		FGenericPlatformStackWalk::EStackWalkFlags::FlagsUsedWhenHandlingEnsure);

	FString Ret(StackTrace);
	FMemory::SystemFree(StackTrace);
	return Ret;
}

} // namespace DcDiagnosticUtils
