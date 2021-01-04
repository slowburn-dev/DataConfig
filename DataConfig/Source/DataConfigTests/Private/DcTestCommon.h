#pragma once

#include "CoreMinimal.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"

class FAutomationTestBase;

FDcResult DcTestPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum);





