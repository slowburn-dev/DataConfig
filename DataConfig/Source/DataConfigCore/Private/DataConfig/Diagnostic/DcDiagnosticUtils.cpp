#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "HAL/PlatformStackWalk.h"

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

FDcScopedDiagHandler::FDcScopedDiagHandler(DiagHandlerType InHandler)
	: Handler(InHandler)
{
	StartDiagCount = DcEnv().Diagnostics.Num();
}

FDcScopedDiagHandler::~FDcScopedDiagHandler()
{
	int EndDiagCount = DcEnv().Diagnostics.Num();
	for (int Ix = StartDiagCount; Ix < EndDiagCount; Ix++)
		Handler(DcEnv().Diagnostics[Ix]);
}

namespace DcDiagnosticUtils
{

void AmendDiagnostic(FDcDiagnostic& Diag, FDcReader* Reader, FDcWriter* Writer)
{
	bool bHasReaderDiag = false;
	bool bHasWriterDiag = false;
	for (FDcDiagnosticHighlight& Highlight : Diag.Highlights)
	{
		if (Reader && Highlight.Owner == Reader)
			bHasReaderDiag = true;
		else if (Writer && Highlight.Owner == Writer)
			bHasWriterDiag = true;
	}

	if (!bHasReaderDiag && Reader)
		Reader->FormatDiagnostic(Diag);
	if (!bHasWriterDiag && Writer)
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

FString SafeObjectName(UObject* Object)
{
	if (Object == nullptr)
		return TEXT("<null object>");
	if (DcPropertyUtils::HeuristicIsPointerInvalid(Object))
		return TEXT("<invalid object pointer bit pattern>");

	return Object->GetName();
}


} // namespace DcDiagnosticUtils
