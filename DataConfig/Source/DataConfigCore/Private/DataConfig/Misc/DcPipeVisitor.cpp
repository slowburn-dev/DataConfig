#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"

FDcPipeVisitor::FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter)
{
	Reader = InReader;
	Writer = InWriter;
}

FDcResult FDcPipeVisitor::PipeVisit()
{
	while (true)
	{
		PreVisit.ExecuteIfBound(this);

		EDcDataEntry PeekEntry;
		DC_TRY(Reader->PeekRead(&PeekEntry));

		bool bWriteOK;
		DC_TRY(Writer->PeekWrite(PeekEntry, &bWriteOK));
		if (!bWriteOK)
			return DC_FAIL(DcDReadWrite, PipeReadWriteMismatch) << PeekEntry;

		if (PeekEntry == EDcDataEntry::Ended)
			return DcOk();
		else
			DC_TRY(DcDeserializeUtils::DispatchPipeVisit(PeekEntry, Reader, Writer));

		PostVisit.ExecuteIfBound(this);
	}
}

