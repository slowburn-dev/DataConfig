#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Property/DcPropertyDatum.h"

#include "DcTestCommon.generated.h"

UENUM()
enum class EDcTestEnum1 : int64
{
	Foo,
	Bar,
	Tard,
};

FDcResult DcTestPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum);





