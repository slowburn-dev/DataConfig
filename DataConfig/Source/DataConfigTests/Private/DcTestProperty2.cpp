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

DC_TEST("DataConfig.Core.Property.Enum")
{
	FDcTestStructEnum1 Source;

	Source.EnumUInt8Field = EDcTestEnum_UInt8::Max;
	Source.EnumUInt16Field = EDcTestEnum_UInt16::Max;
	Source.EnumUInt32Field = EDcTestEnum_UInt32::Max;
	Source.EnumUInt64Field = EDcTestEnum_UInt64::Max;

	Source.EnumInt8Field = EDcTestEnum_Int8::Max;
	Source.EnumInt16Field = EDcTestEnum_Int16::Max;
	Source.EnumInt32Field = EDcTestEnum_Int32::Max;
	Source.EnumInt64Field = EDcTestEnum_Int64::Max;

	Source.EnumFlagField = EDcTestEnum_Flag::Alpha | EDcTestEnum_Flag::Gamma;

	FDcTestStructEnum1 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStructEnum1::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStructEnum1::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStructEnum1 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStructEnum1 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}


