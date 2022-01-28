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

template<typename TThunk>
FDcResult DcTestPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum, const TThunk& Func)
{
	FDcPropertyReader Reader(FromDatum);
	FDcPropertyWriter Writer(ToDatum);
	DC_TRY(Func(Reader, Writer));
	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	return RoundtripVisit.PipeVisit();
}

FORCEINLINE FDcResult DcTestPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum)
{
	return DcTestPropertyRoundtrip(Fixture, FromDatum, ToDatum, [](FDcPropertyReader&, FDcPropertyWriter&){
		return DcOk();
	});
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

DATACONFIGEXTRA_API FString DcReindentStringLiteral(FString Str, FString* Prefix = nullptr);




