#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"

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


FString DcGetFixturePath(const FString& Str);




