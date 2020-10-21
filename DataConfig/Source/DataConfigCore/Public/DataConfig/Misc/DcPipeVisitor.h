#pragma once

#include "DataConfig/DcTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FDcPipeVisitor
{
	FDcReader* Reader;
	FDcWriter* Writer;

	FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter);

	FDcResult PipeVisit();
};

}	// namespace DataConfig






