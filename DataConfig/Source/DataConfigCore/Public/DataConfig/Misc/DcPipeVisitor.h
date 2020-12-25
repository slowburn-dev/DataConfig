#pragma once

#include "DataConfig/DcTypes.h"

struct FDcPipeVisitor;
DECLARE_DELEGATE_OneParam(FDcPipeVisitEvent, FDcPipeVisitor*);

struct FDcReader;
struct FDcWriter;

struct DATACONFIGCORE_API FDcPipeVisitor
{
	FDcReader* Reader;
	FDcWriter* Writer;

	FDcPipeVisitEvent PreVisit;
	FDcPipeVisitEvent PostVisit;

	FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter);

	FDcResult PipeVisit();

};


