#pragma once

#include "DataConfig/DcTypes.h"

enum class EPipeVisitControl : uint8
{
	Pass,				//	do nothing
	BreakVisit,			//	early exit visit
	SkipContinue,		//	skip next pipe visit, usually means read write is handled here
};

struct FDcPipeVisitor;
DECLARE_DELEGATE_RetVal_TwoParams(FDcResult, FDcPipeVisitEvent, FDcPipeVisitor*, EPipeVisitControl&);
DECLARE_DELEGATE_RetVal_ThreeParams(FDcResult, FDcPipeVisitPeekEvent, FDcPipeVisitor*, EDcDataEntry, EPipeVisitControl&);

struct FDcReader;
struct FDcWriter;

struct DATACONFIGCORE_API FDcPipeVisitor
{
	FDcReader* Reader;
	FDcWriter* Writer;

	FDcPipeVisitEvent PreVisit;
	FDcPipeVisitPeekEvent PeekVisit;
	FDcPipeVisitPeekEvent PostVisit;

	FDcPipeVisitor(FDcReader* InReader, FDcWriter* InWriter);

	FDcResult PipeVisit();
};

