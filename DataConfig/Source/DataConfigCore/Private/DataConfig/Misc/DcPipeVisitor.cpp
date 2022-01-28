#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

namespace DcPipeVisitorDetails
{

static FDcResult ExecutePipeVisit(FDcPipeVisitor* Self)
{
	while (true)
	{
		EPipeVisitControl Control = EPipeVisitControl::Pass;

		if (Self->PreVisit.IsBound())
		{
			DC_TRY(Self->PreVisit.Execute(Self, Control));
			if (Control == EPipeVisitControl::BreakVisit)
				return DcOk();
			if (Control == EPipeVisitControl::SkipContinue)
				continue;
		}

		EDcDataEntry PeekEntry;
		DC_TRY(Self->Reader->PeekRead(&PeekEntry));

		if (Self->PeekVisit.IsBound())
		{
			DC_TRY(Self->PeekVisit.Execute(Self, PeekEntry, Control));
			if (Control == EPipeVisitControl::BreakVisit)
				return DcOk();
			if (Control == EPipeVisitControl::SkipContinue)
				continue;
		}

		{
			bool bWriteOK;
			DC_TRY(Self->Writer->PeekWrite(PeekEntry, &bWriteOK));
			if (!bWriteOK)
				return DC_FAIL(DcDReadWrite, PipeReadWriteMismatch) << PeekEntry;

			if (PeekEntry == EDcDataEntry::Ended)
				return DcOk();
			else
				DC_TRY(DcSerDeUtils::DispatchPipeVisit(PeekEntry, Self->Reader, Self->Writer));
		}

		if (Self->PostVisit.IsBound())
		{
			DC_TRY(Self->PostVisit.Execute(Self, PeekEntry, Control));
			if (Control == EPipeVisitControl::BreakVisit)
				return DcOk();
			if (Control == EPipeVisitControl::SkipContinue)
				continue;
		}
	}
}

} // namespace DcPipeVisitorDetails


FDcPipeVisitor::FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter)
{
	Reader = InReader;
	Writer = InWriter;
}

FDcResult FDcPipeVisitor::PipeVisit()
{
	FDcResult Result = DcPipeVisitorDetails::ExecutePipeVisit(this);
	if (!Result.Ok()
		&& !DcEnv().bExpectFail)
		DcDiagnosticUtils::AmendDiagnostic(DcEnv().GetLastDiag(), Reader, Writer);

	return Result;
}


