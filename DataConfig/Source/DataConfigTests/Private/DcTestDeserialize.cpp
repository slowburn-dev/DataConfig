#include "DcTestDeserialize.h"
#include "DcTestProperty.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

template<typename TThunk>
FDcResult _DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum, const TThunk& Func)
{
	FDcDeserializer Deserializer;
	DcSetupJsonDeserializeHandlers(Deserializer);

	FDcPropertyWriter Writer(Datum);
	FDcDeserializeContext Ctx;
	Ctx.Reader = Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &Deserializer;
	Ctx.Properties.Push(Datum.Property);
	Func(Ctx);
	Ctx.Prepare();

	return Deserializer.Deserialize(Ctx);
}

static FDcResult _DeserializeJsonInto(FDcReader* Reader, FDcPropertyDatum Datum)
{
	return _DeserializeJsonInto(Reader, Datum, [](FDcDeserializeContext&){ /*pass*/ });
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

DC_TEST("DataConfig.Core.Deserializer.InlineSubObject")
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"ShapeField1" :  {
				"$type" : "DcShapeBox",
				"ShapeName" : "Box1",
				"Height" : 17.5,
				"Width" : 1.9375
			},
			"ShapeField2" : {
				"$type" : "DcShapeSquare",
				"ShapeName" : "Square1",
				"Radius" : 1.75,
			},
			"ShapeField3" : null
		}

	)");
	Reader.SetNewString(*Str);

	FDcTestStructShapeContainer1 Dest;	//	note that Dest fields are all uninitialized atm
	FDcPropertyDatum DestDatum(FDcTestStructShapeContainer1::StaticStruct(), &Dest);

	FDcTestStructShapeContainer1 Expect;

	UDcShapeBox* Shape1 = NewObject<UDcShapeBox>();
	Shape1->ShapeName = TEXT("Box1");
	Shape1->Height = 17.5;
	Shape1->Width = 1.9375;
	Expect.ShapeField1 = Shape1;

	UDcShapeSquare* Shape2 = NewObject<UDcShapeSquare>();
	Shape2->ShapeName = TEXT("Square1");
	Shape2->Radius = 1.75;
	Expect.ShapeField2 = Shape2;

	Expect.ShapeField3 = nullptr;

	FDcPropertyDatum ExpectDatum(FDcTestStructShapeContainer1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", _DeserializeJsonInto(&Reader, DestDatum, [](FDcDeserializeContext& Ctx){
		Ctx.Objects.Push(GetTransientPackage());
	}));
	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserializer.ObjectRef")
{
	FDcJsonReader Reader;
	FString Str = TEXT(R"(

		{
			"ObjField1" : "Object'/Script/DataConfigTests'",
			"ObjField2" : null
		}

	)");
	Reader.SetNewString(*Str);

	FDcTestStructObjectRef1 Dest;	//	note that Dest fields are all uninitialized atm
	FDcPropertyDatum DestDatum(FDcTestStructObjectRef1::StaticStruct(), &Dest);

	FDcTestStructObjectRef1 Expect;

	Expect.ObjField1 = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
	Expect.ObjField2 = nullptr;

	FDcPropertyDatum ExpectDatum(FDcTestStructObjectRef1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStructObjectRef1", _DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructObjectRef1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}
