#include "DcTestCommon.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Property/DcPropertyDatum.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Misc/DcPipeVisitor.h"
#include "DataConfig/Automation/DcAutomation.h"

static FDcResult _DcPropertyRoundtrip(FAutomationTestBase* Fixture, FDcPropertyDatum FromDatum, FDcPropertyDatum ToDatum)
{
	FDcPropertyReader Reader(FromDatum);
	FDcPropertyWriter Writer(ToDatum);
	FDcPipeVisitor RoundtripVisit(&Reader, &Writer);
	return RoundtripVisit.PipeVisit();
}

DC_TEST("DataConfig.Core.Property.Primitive1")
{
	FDcTestStruct1 Source;

	Source.BoolField = true;
	Source.NameField = TEXT("SomeName");
	Source.StringField = TEXT("Some String");
	Source.TextField = FText(NSLOCTEXT("DcAdhocs", "T1", "Some Text"));
	Source.EnumField = EDcTestEnum1::Bar;

	Source.FloatField = 12.3f;
	Source.DoubleField = 23.4;

	Source.Int8Field = 123;
	Source.Int16Field = 234;
	Source.Int32Field = 34567;
	Source.Int64Field = 456789;

	Source.UInt8Field = -123;
	Source.UInt16Field = -234;
	Source.UInt32Field = -34567;
	Source.UInt64Field = -456789;

	FDcTestStruct1 Dest;

	UTEST_OK("FDcTestStruct1 roundtrip", _DcPropertyRoundtrip(
		this,
		FDcPropertyDatum(FDcTestStruct1::StaticStruct(), &Source),
		FDcPropertyDatum(FDcTestStruct1::StaticStruct(), &Dest)
	));

	//UTEST_EQUAL("FDcTestStruct1 roundtrip equal", Source, Dest);

	return true;
}

