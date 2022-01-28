#include "DcTestSerDe.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
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
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStruct1 Expect;
	Expect.MakeFixture();

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStruct1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
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
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructEnumFlag1 Expect;
	Expect.MakeFixture();
	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructEnumFlag1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
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
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructShapeContainer1 Expect;
	Expect.MakeFixture();
	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
	[](FDcDeserializeContext& Ctx){
		Ctx.Objects.Push(GetTransientPackage());
	}));
	UTEST_OK("Deserialize into FDcTestStructShapeContainer1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.ObjectRef")
{
	FString Str = TEXT(R"(

		{
			"ObjField1" : "Package'/Script/DataConfigTests'",
			"ObjField2" : "'/Script/DataConfigTests'",
			"ObjField3" : null
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructObjectRef1 Dest;	//	note that Dest fields are all uninitialized atm
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructObjectRef1 Expect;

	Expect.ObjField1 = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
	Expect.ObjField2 = Expect.ObjField1;
	Expect.ObjField3 = nullptr;

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructObjectRef1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
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
			"StructArray" : [
				{
					"Name" : "One",
					"Index" : 1
				},
				{
					"Name" : "Two",
					"Index" : 2
				},
				{
					"Name" : "Three",
					"Index" : 3
				}
			],
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
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStruct3 Expect;
	Expect.MakeFixtureNoStructMap();
	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStruct3", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
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
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructSubClass1 Expect;

	Expect.StructSubClassField1 = nullptr;
	Expect.StructSubClassField2 = UScriptStruct::StaticClass();
	Expect.StructSubClassField3 = UDynamicClass::StaticClass();

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructSubClass1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructSubClass1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));


	FString BadStr = TEXT(R"(

		{
			"StructSubClassField1" : "Object",
		}

	)");

	UTEST_OK("Deserialize into FDcTestStructSubClass1 Fail", Reader.SetNewString(*BadStr));
	UTEST_DIAG("Deserialize into FDcTestStructSubClass1 Fail", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum),
		DcDSerDe, ClassLhsIsNotChildOfRhs);

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.ObjRefs")
{
	FString Str = TEXT(R"(

		{
			"ObjectField1" : "'/Script/DataConfigTests'",
			"ObjectField2" : null,
			"SoftField1" : "'/Script/DataConfigTests'",
			"SoftField2" : null,
			"WeakField1" : "'/Script/DataConfigTests'",
			"WeakField2" : null,
			"LazyField1" : "'/Script/DataConfigTests'",
			"LazyField2" : null,
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructRefs1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructRefs1 Expect;
	Expect.MakeFixture();

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructRefs1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructRefs1", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.ClassRefs")
{
	FString Str = TEXT(R"(

		{
			"RawClassField1" : "DynamicClass",
			"RawClassField2" : null,
			"SubClassField1" : "DynamicClass",
			"SubClassField2" : null,
			"SoftClassField1" : "DynamicClass",
			"SoftClassField2" : null,
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructRefs2 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructRefs2 Expect;
	Expect.MakeFixture();

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize into FDcTestStructRefs2", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
	UTEST_OK("Deserialize into FDcTestStructRefs2", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

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
		FDcPropertyDatum DestDatum(&Dest);

		UTEST_DIAG("Deserialize Fails", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum), DcDReadWrite, CantFindPropertyByName);
	}


	return true;
}

DC_TEST("DataConfig.Core.Deserialize.SkipMetas")
{
	FString Str = TEXT(R"(

		{
			"$meta1" : "Foo",
			"$meta2" : 123,
			"$meta3" : [1,2,3, "four", "five", [[[]]], [{}, {}, {}]],
			"$meta4" : { "one": "uno", "two": 2, "nest" : {}, "nest2": []},
			"$meta5" : null,
			"IntField" : 253,
			"StrField" : "Foo"
		}

	)");
	FDcJsonReader Reader(Str);

	UDcTestClass1* Dest = NewObject<UDcTestClass1>();
	FDcPropertyDatum DestDatum(Dest);

	UDcTestClass1* Expect = NewObject<UDcTestClass1>();
	Expect->IntField = 253;
	Expect->StrField = "Foo";

	FDcPropertyDatum ExpectDatum(Expect);

	UTEST_OK("Deserialize SkipMetas", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));
	UTEST_OK("Deserialize SkipMetas", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}


