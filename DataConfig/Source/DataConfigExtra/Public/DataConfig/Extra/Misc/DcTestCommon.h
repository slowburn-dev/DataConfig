#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Writer/DcNoopWriter.h"
#include "DataConfig/Writer/DcPrettyPrintWriter.h"

class FAutomationTestBase;

FORCEINLINE FDcResult DcPropertyPipeVisit(FDcPropertyReader& Reader, FDcPropertyWriter& Writer)
{
	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	return RoundtripVisit.PipeVisit();
}

FORCEINLINE FDcResult DcPropertyPipeVisit(FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum)
{
	FDcPropertyReader Reader(FromDatum);
	FDcPropertyWriter Writer(ToDatum);
	return DcPropertyPipeVisit(Reader, Writer);
}

FORCEINLINE FDcResult DcNoopPipeVisit(FDcReader* Reader)
{
	FDcNoopWriter Writer{};
	FDcPipeVisitor PipeVisitor(Reader, &Writer);
	return PipeVisitor.PipeVisit();
}

FORCEINLINE FDcResult DcDumpPipeVisit(FDcReader* Reader)
{
	FOutputDevice& WarnOut = (FOutputDevice&)*GWarn;
	FDcPrettyPrintWriter Writer(WarnOut);

	FDcPipeVisitor PipeVisitor(Reader, &Writer);
	return PipeVisitor.PipeVisit();
}

DATACONFIGEXTRA_API FString DcGetFixturePath(const FString& Str);

DATACONFIGEXTRA_API FDcResult DcPropertyPipeVisitAndTestEqual(FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum);



