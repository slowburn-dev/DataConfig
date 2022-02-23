#include "DcTestSerDe.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"

DC_TEST("DataConfig.Core.Serialize.Primitive1")
{
	FDcTestStruct1 Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

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
			"UInt64Field" : 1524523
		}

	)"));

	UTEST_OK("Serialize FDcTestStruct1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStruct1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.EnumFlags")
{
	{
		FDcTestStructEnumFlag1 Value;
		Value.MakeFixture();
		FDcPropertyDatum Datum(&Value);

		FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

			{
				"EnumFlagField1" : [],
				"EnumFlagField2" : [
					"One",
					"Three",
					"Five"
				]
			}

		)"));

		FDcJsonWriter Writer;

		UTEST_OK("Serialize FDcTestStructEnumFlag1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Serialize FDcTestStructEnumFlag1 into Json", Writer.Sb.ToString(), ExpectStr);

	}

	{
		FDcTestStructEnumFlag1 Value;
		Value.EnumFlagField1 = (EDcTestEnumFlag)253;
		FDcPropertyDatum Datum(&Value);

		FDcJsonWriter Writer;
		UTEST_DIAG("Serialize FDcTestStructEnumFlag1 into Json",
			DcAutomationUtils::SerializeInto(&Writer, Datum),
			DcDSerDe, EnumBitFlagsNotFullyMasked);
	}

	return true;
}

DC_TEST("DataConfig.Core.Serialize.Containers")
{
	FDcTestStruct3 Value;
	Value.MakeFixtureNoStructMap();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"StringArray" : [
				"Foo",
				"Bar",
				"Baz"
			],
			"StringSet" : [
				"Doo",
				"Dar",
				"Daz"
			],
			"StringMap" : {
				"One" : "1",
				"Two" : "2",	
				"Three" : "3"
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
			"StructMap" : {}
		}

	)"));

	UTEST_OK("Serialize FDcTestStruct1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStruct1 into Json", Writer.Sb.ToString(), ExpectStr);


	return true;
}

DC_TEST("DataConfig.Core.Serialize.InlineSubObject")
{
	FDcTestStructShapeContainer1 Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"ShapeField1" : {
				"$type" : "DcShapeBox",
				"Height" : 17.5,
				"Width" : 1.9375,
				"ShapeName" : "Box1"
			},
			"ShapeField2" : {
				"$type" : "DcShapeSquare",
				"Radius" : 1.75,
				"ShapeName" : "Square1"
			},
			"ShapeField3" : null
		}

	)"));

	UTEST_OK("Serialize FDcTestStructShapeContainer1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStructShapeContainer1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.ObjectRef")
{
	FDcTestStructObjectRef1 Value;

	Value.ObjField1 = FindObject<UPackage>(ANY_PACKAGE, TEXT("/Script/DataConfigTests"), true);
	Value.ObjField2 = Value.ObjField1;
	Value.ObjField3 = nullptr;

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"ObjField1" : "'/Script/DataConfigTests'",
			"ObjField2" : "'/Script/DataConfigTests'",
			"ObjField3" : null
		}

	)"));

	UTEST_OK("Serialize FDcTestStructObjectRef1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStructObjectRef1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.ObjRefs")
{
	FDcTestStructRefs1 Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"ObjectField1" : "'/Script/DataConfigTests'",
			"ObjectField2" : null,
			"SoftField1" : "'/Script/DataConfigTests'",
			"SoftField2" : null,
			"WeakField1" : "'/Script/DataConfigTests'",
			"WeakField2" : null,
			"LazyField1" : "'/Script/DataConfigTests'",
			"LazyField2" : null
		}

	)"));

	UTEST_OK("Serialize FDcTestStructRefs1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStructRefs1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.SubClass")
{
	FDcTestStructSubClass1 Value;

	Value.StructSubClassField1 = nullptr;
	Value.StructSubClassField2 = UScriptStruct::StaticClass();
	Value.StructSubClassField3 = UFunction::StaticClass();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"StructSubClassField1" : null,
			"StructSubClassField2" : "ScriptStruct",
			"StructSubClassField3" : "Function"
		}

	)"));

	UTEST_OK("Serialize FDcTestStructSubClass1 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStructSubClass1 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.ClassRefs")
{
	FDcTestStructRefs2 Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcReindentStringLiteral(TEXT(R"(

		{
			"RawClassField1" : "DynamicClass",
			"RawClassField2" : null,
			"SubClassField1" : "DynamicClass",
			"SubClassField2" : null,
			"SoftClassField1" : "DynamicClass",
			"SoftClassField2" : null
		}

	)"));

	UTEST_OK("Serialize FDcTestStructRefs2 into Json", DcAutomationUtils::SerializeInto(&Writer, Datum));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize FDcTestStructRefs2 into Json", Writer.Sb.ToString(), ExpectStr);

	return true;
}



