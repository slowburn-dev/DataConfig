#include "DcTestDeserialize.h"
#include "DcTestProperty.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

static FDcResult _DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum)
{
	FDcDeserializer Deserializer;
	DcSetupJsonDeserializeHandlers(Deserializer);

	FDcPropertyWriter Writer(Datum);
	FDcDeserializeContext Ctx;
	Ctx.Reader = Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(Datum.Property);
	Ctx.Prepare();

	return Deserializer.Deserialize(Ctx);
}

DC_TEST("DataConfig.Core.Deserializer.Primitive1")
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"BoolField" : true,
			"NameField" : "AName",
			"StringField" : "AStr",
			"TextField" : "AText",
			"EnumField" : "Tard",

			"FloatField" : 17.5,
			"DoubleField" : 19.375,

			"Int8Field" : -43,
			"Int16Field" : -2243,
			"Int32Field" : -23415,
			"Int64Field" : -1524523,

			"UInt8Field" : 213,
			"UInt16Field" : 2243,
			"UInt32Field" : 23415,
			"UInt64Field" : 1524523,
		}

	)");
	Reader.SetNewString(*Str);

	FDcTestStruct1 Dest;
	FDcPropertyDatum DestDatum(FDcTestStruct1::StaticStruct(), &Dest);

	FDcTestStruct1 Expect;
	Expect.BoolField = true;
	Expect.NameField = TEXT("AName");
	Expect.StringField = TEXT("AStr");
	Expect.TextField = FText::FromString(TEXT("AText"));
	Expect.EnumField = EDcTestEnum1::Tard;

	Expect.FloatField = 17.5f;
	Expect.DoubleField = 19.375;

	Expect.Int8Field = -43;
	Expect.Int16Field = -2243;
	Expect.Int32Field = -23415;
	Expect.Int64Field = -1524523;

	Expect.UInt8Field = 213;
	Expect.UInt16Field = 2243,
	Expect.UInt32Field = 23415;
	Expect.UInt64Field = 1524523;

	FDcPropertyDatum ExpectDatum(FDcTestStruct1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStruct1", _DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStruct1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserializer.EnumFlags")
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"EnumFlagField1" : [],
			"EnumFlagField2" : ["One", "Three", "Five"],
		}

	)");
	Reader.SetNewString(*Str);


	FDcTestStructEnumFlag1 Dest;
	FDcPropertyDatum DestDatum(FDcTestStructEnumFlag1::StaticStruct(), &Dest);

	FDcTestStructEnumFlag1 Expect;
	Expect.EnumFlagField1 = EDcTestEnumFlag::None;
	Expect.EnumFlagField2 = EDcTestEnumFlag::One | EDcTestEnumFlag::Three | EDcTestEnumFlag::Five;
	FDcPropertyDatum ExpectDatum(FDcTestStructEnumFlag1::StaticStruct(), &Expect);


#if !WITH_METADATA
	UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("EDcTestEnumFlag"), true);
	check(EnumClass);
	DcAutomationUtils::AmendMetaData(EnumClass, TEXT("Bitflags"), TEXT(""));
	DcAutomationUtils::AmendMetaData(EnumClass, TEXT("Bitflags"), TEXT(""));
#endif

	UTEST_OK("Deserialize into FDcTestStructEnumFlag1", _DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructEnumFlag1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

