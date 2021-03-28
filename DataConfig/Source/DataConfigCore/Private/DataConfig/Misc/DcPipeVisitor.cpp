#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

namespace DcPipeVisitorDetails
{

static FDcResult ExecutePipeVisit(FDcPipeVisitor* Self)
{
	while (true)
	{
		Self->PreVisit.ExecuteIfBound(Self);

		EDcDataEntry PeekEntry;
		DC_TRY(Self->Reader->PeekRead(&PeekEntry));

		bool bWriteOK;
		DC_TRY(Self->Writer->PeekWrite(PeekEntry, &bWriteOK));
		if (!bWriteOK)
			return DC_FAIL(DcDReadWrite, PipeReadWriteMismatch) << PeekEntry;

		if (PeekEntry == EDcDataEntry::Ended)
			return DcOk();
		else
			DC_TRY(DcDeserializeUtils::DispatchPipeVisit(PeekEntry, Self->Reader, Self->Writer));

		Self->PostVisit.ExecuteIfBound(Self);
	}
}

}	// namespace DcPipeVisitorDetails


FDcPipeVisitor::FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter)
{
	Reader = InReader;
	Writer = InWriter;
}

FDcResult FDcPipeVisitor::PipeVisit()
{
	FDcResult Result = DcPipeVisitorDetails::ExecutePipeVisit(this);
	if (!Result.Ok())
		DcDiagnosticUtils::AmendDiagnostic(DcEnv().GetLastDiag(), Reader, Writer);

	return Result;
}


