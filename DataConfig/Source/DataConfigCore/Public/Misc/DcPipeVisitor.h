#pragma once

#include "DcTypes.h"

namespace DataConfig
{

struct DATACONFIGCORE_API FPipeVisitor
{
	FReader* Reader;
	FWriter* Writer;

	FPipeVisitor(FReader* InReader, FWriter* InWriter);

	FResult PipeVisit();
};

}	// namespace DataConfig






