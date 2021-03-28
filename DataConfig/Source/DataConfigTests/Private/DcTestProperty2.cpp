#include "DcTestProperty2.h"
#include "DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

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

	UTEST_TRUE("FDcTestStructEnum1 roundtrip equal", Dest.EnumFlagField == (EDcTestEnum_Flag::Alpha | EDcTestEnum_Flag::Gamma));

	return true;
}

DC_TEST("DataConfig.Core.Property.Config")
{
	FDcTestStructSimple Source;

	Source.NameField = TEXT("Named");
	Source.StrField = TEXT("Stred");

	FDcTestStructSimple Dest;

	FDcPropertyDatum SourceDatum(FDcTestStructSimple::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStructSimple::StaticStruct(), &Dest);

	UTEST_OK("Property Reader/Writer Config", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum, [](FDcPropertyReader& Reader, FDcPropertyWriter& Writer){

		FDcPropertyConfig IgnoreStrConfig = FDcPropertyConfig::MakeDefault();
		IgnoreStrConfig.ProcessPropertyPredicate.BindLambda([](FProperty* Property){
			return !Property->IsA<FStrProperty>();
		});

		DC_TRY(Reader.SetConfig(IgnoreStrConfig));
		DC_TRY(Writer.SetConfig(IgnoreStrConfig));
		return DcOk();
	}));

	UTEST_TRUE("Property Reader/Writer Config", Dest.NameField == TEXT("Named"));
	UTEST_TRUE("Property Reader/Writer Config", Dest.StrField.IsEmpty());

	return true;
}

DC_TEST("DataConfig.Core.Property.DefaultValue")
{
	FDcTestStructDefaultValue2 Source;

	Source.StructsArray.Emplace(FDcTestStructDefaultValue1{1});
	Source.StructsArray.Emplace(FDcTestStructDefaultValue1{2});
	Source.StructsArray.Emplace(FDcTestStructDefaultValue1{3});

	Source.StructsMap.Emplace(TEXT("One"), FDcTestStructDefaultValue1{1});
	Source.StructsMap.Emplace(TEXT("Two"), FDcTestStructDefaultValue1{2});
	Source.StructsMap.Emplace(TEXT("Three"), FDcTestStructDefaultValue1{3});

	FDcTestStructDefaultValue2 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStructDefaultValue2::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStructDefaultValue2::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStructDefaultValue2 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStructDefaultValue2 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}

DC_TEST("DataConfig.Core.Property.EnumNamespace")
{
	FDcTestStructEnum2 Source;

	Source.EnumNamespaced1 = EDcTestEnum_Namespaced::Cero;
	Source.EnumNamespaced2 = EDcTestEnum_Namespaced::Tres;

	FDcTestStructEnum2 Dest;

	FDcPropertyDatum SourceDatum(FDcTestStructEnum2::StaticStruct(), &Source);
	FDcPropertyDatum DestDatum(FDcTestStructEnum2::StaticStruct(), &Dest);

	UTEST_OK("FDcTestStructEnum2 roundtrip", DcTestPropertyRoundtrip(this, SourceDatum, DestDatum));
	UTEST_OK("FDcTestStructEnum2 roundtrip equal", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	return true;
}
