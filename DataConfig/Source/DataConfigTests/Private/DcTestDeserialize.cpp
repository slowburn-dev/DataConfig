#include "DcTestDeserialize.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

DC_TEST("DataConfig.Core.Deserialize.Primitive1")
{
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
	FDcJsonReader Reader(Str);

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

	UTEST_OK("Deserialize into FDcTestStruct1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStruct1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.EnumFlags")
{
	FString Str = TEXT(R"(

		{
			"EnumFlagField1" : [],
			"EnumFlagField2" : ["One", "Three", "Five"],
		}

	)");
	FDcJsonReader Reader(Str);


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

	UTEST_OK("Deserialize into FDcTestStructEnumFlag1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructEnumFlag1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.InlineSubObject")
{
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
	FDcJsonReader Reader(Str);

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

	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer&, FDcDeserializeContext& Ctx){
		Ctx.Objects.Push(GetTransientPackage());
	}));
	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.ObjectRef")
{
	FString Str = TEXT(R"(

		{
			"ObjField1" : "Object'/Script/DataConfigTests'",
			"ObjField2" : null
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructObjectRef1 Dest;	//	note that Dest fields are all uninitialized atm
	FDcPropertyDatum DestDatum(FDcTestStructObjectRef1::StaticStruct(), &Dest);

	FDcTestStructObjectRef1 Expect;

	Expect.ObjField1 = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
	Expect.ObjField2 = nullptr;

	FDcPropertyDatum ExpectDatum(FDcTestStructObjectRef1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStructObjectRef1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructObjectRef1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}


DC_TEST("DataConfig.Core.Deserialize.Containers")
{
	FString Str = TEXT(R"(

		{
			"StringArray" : [
				"Foo", "Bar", "Baz"
			],
			"StringSet" : [
				"Doo", "Dar", "Daz"
			],
			"StringMap" : {
				"One": "1",
				"Two": "2",	
				"Three": "3",
			},
			"StructSet" : [
				{
					"Name" : "One",
					"Index" : 1,
				},
				{
					"Name" : "Two",
					"Index" : 2,
				},
				{
					"Name" : "Three",
					"Index" : 3,
				}
			],
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStruct3 Dest;
	FDcPropertyDatum DestDatum(FDcTestStruct3::StaticStruct(), &Dest);

	FDcTestStruct3 Expect;

	Expect.StringArray.Add(TEXT("Foo"));
	Expect.StringArray.Add(TEXT("Bar"));
	Expect.StringArray.Add(TEXT("Baz"));

	Expect.StringSet.Add(TEXT("Doo"));
	Expect.StringSet.Add(TEXT("Dar"));
	Expect.StringSet.Add(TEXT("Daz"));

	Expect.StringMap.Add(TEXT("One"), TEXT("1"));
	Expect.StringMap.Add(TEXT("Two"), TEXT("2"));
	Expect.StringMap.Add(TEXT("Three"), TEXT("3"));

	Expect.StructSet.Add({TEXT("One"), 1});
	Expect.StructSet.Add({TEXT("Two"), 2});
	Expect.StructSet.Add({TEXT("Three"), 3});

	FDcPropertyDatum ExpectDatum(FDcTestStruct3::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStruct3", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStruct3", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.SubClass")
{
	FString Str = TEXT(R"(

		{
			"StructSubClassField1" : null,
			"StructSubClassField2" : "ScriptStruct",
			"StructSubClassField3" : "DynamicClass",
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructSubClass1 Dest;
	FDcPropertyDatum DestDatum(FDcTestStructSubClass1::StaticStruct(), &Dest);

	FDcTestStructSubClass1 Expect;

	Expect.StructSubClassField1 = nullptr;
	Expect.StructSubClassField2 = UScriptStruct::StaticClass();
	Expect.StructSubClassField3 = UDynamicClass::StaticClass();

	FDcPropertyDatum ExpectDatum(FDcTestStructSubClass1::StaticStruct(), &Expect);

	UTEST_OK("Deserialize into FDcTestStructSubClass1", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructSubClass1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));


	FString BadStr = TEXT(R"(

		{
			"StructSubClassField1" : "Object",
		}

	)");

	UTEST_OK("Deserialize into FDcTestStructSubClass1 Fail", Reader.SetNewString(*BadStr));
	UTEST_DIAG("Deserialize into FDcTestStructSubClass1 Fail", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum),
		DcDDeserialize, ClassLhsIsNotChildOfRhs);

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.Fails1")
{

	{
		FString Str = TEXT(R"(

			{
				"WhatField" : "Doesnt Exist",
			}

		)");
		FDcJsonReader Reader(Str);

		FDcTestStruct1 Dest;
		FDcPropertyDatum DestDatum(FDcTestStruct1::StaticStruct(), &Dest);

		UTEST_DIAG("Deserialize Fails", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum), DcDReadWrite, CantFindPropertyByName);
	}


	return true;
}

