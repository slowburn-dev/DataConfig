#pragma once

#include "DataConfig/DcTypes.h"

struct DATACONFIGCORE_API FDcPipeVisitor
{
	FDcReader* Reader;
	FDcWriter* Writer;

	FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter);

	FDcResult PipeVisit();
};


