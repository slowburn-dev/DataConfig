#include "DcTestProperty2.h"
#include "DcTestCommon.h"

DC_TEST("DataConfig.Core.Property.NestedStruct")
{
	FDcTestStructNest4 Source;

	auto& Nest3 = Source.StructArrayField.AddDefaulted_GetRef();
	auto& Nest1 = Nest3.StructField.StrStructMapField.Emplace(TEXT("Foo"), FDcTestStructNest1());
	Nest1.NameField = TEXT("Nest1");
	Nest1.StructField.NameField = TEXT("Simple");
	Nest1.StructField.StrField = TEXT("Struct");

	FDcTestStructNest4 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStructNest4::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStructNest4::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStructNest4 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStructNest4 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

