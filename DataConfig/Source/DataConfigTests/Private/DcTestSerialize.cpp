#include "DcTestProperty2.h"
#include "DcTestSerDe.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Property/DcPropertyReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"

DC_TEST("DataConfig.Core.Serialize.Primitive1")
{
	FDcTestStruct1 Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

		FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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
			"StructMap" : [] 
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

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

	Value.ObjField1 = FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests"), true);
	Value.ObjField2 = Value.ObjField1;
	Value.ObjField3 = nullptr;

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

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


DC_TEST("DataConfig.Core.Serialize.NonStructClassRoots")
{
	FString SrcStr = TEXT("These are my twisted words");

	uint64 SrcU64Arr[2] = {TNumericLimits<uint64>::Min(), TNumericLimits<uint64>::Max()};

	TArray<FDcTestStructSimple> SrcArr = {
		{TEXT("Foo"), TEXT("Bar")},
		{TEXT("Doo"), TEXT("Dar")},
		{TEXT("Goo"), TEXT("Gar")},
	};

	TSet<EDcTestEnum1> SrcSet = {
		EDcTestEnum1::Foo,
		EDcTestEnum1::Bar,
		EDcTestEnum1::Tard,
	};

	TMap<FString, int> SrcMap = {
		{TEXT("One"), 1},
		{TEXT("Two"), 2},
		{TEXT("Three"), 3},
	};

	using namespace DcPropertyUtils;

	auto StrProp = FDcPropertyBuilder::Str().LinkOnScope();		
	auto U64ArrProp = FDcPropertyBuilder::UInt64().ArrayDim(2).LinkOnScope();

	auto ArrProp = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::Struct(FDcTestStructSimple::StaticStruct())
		).LinkOnScope();

	auto SetProp = FDcPropertyBuilder::Set(
			FDcPropertyBuilder::Enum(
				StaticEnum<EDcTestEnum1>(),
				FDcPropertyBuilder::Int64()
				)
			).LinkOnScope();

	auto MapProp = FDcPropertyBuilder::Map(
			FDcPropertyBuilder::Str(),
			FDcPropertyBuilder::Int()
		).LinkOnScope();

	auto _CheckSerializeEqual = [](FDcAutomationBase* Self, FString ExpectStr, FDcPropertyDatum Datum) -> bool
	{
		FDcJsonWriter Writer;
		if (!Self->TestOk("Serialize NonStructClassRoots", DcAutomationUtils::SerializeInto(&Writer, Datum)))
			return false;
		Writer.Sb << TCHAR('\n');

		if (!Self->TestEqual("Serialize NonStructClassRoots", Writer.Sb.ToString(), ExpectStr))
			return false;

		return true;
	};

	UTEST_TRUE("Serialize NonStructClassRoots",  _CheckSerializeEqual(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(
			"These are my twisted words"
		)")),
		FDcPropertyDatum(StrProp.Get(), &SrcStr)
		));

	UTEST_TRUE("Serialize NonStructClassRoots",  _CheckSerializeEqual(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(
			[
				0,
				18446744073709551615
			]
		)")),
		FDcPropertyDatum(U64ArrProp.Get(), &SrcU64Arr)
		));

	UTEST_TRUE("Serialize NonStructClassRoots",  _CheckSerializeEqual(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(
			[
				{
					"NameField" : "Foo",
					"StrField" : "Bar"
				},
				{
					"NameField" : "Doo",
					"StrField" : "Dar"
				},
				{
					"NameField" : "Goo",
					"StrField" : "Gar"
				}
			]		
		)")),
		FDcPropertyDatum(ArrProp.Get(), &SrcArr)
		));

	UTEST_TRUE("Serialize NonStructClassRoots",  _CheckSerializeEqual(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(
			[
				"Foo",
				"Bar",
				"Tard"
			]		
		)")),
		FDcPropertyDatum(SetProp.Get(), &SrcSet)
		));

	UTEST_TRUE("Serialize NonStructClassRoots",  _CheckSerializeEqual(
		this,
		DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(
			{
				"One" : 1,
				"Two" : 2,
				"Three" : 3
			}
		)")),
		FDcPropertyDatum(MapProp.Get(), &SrcMap)
		));

	return true;
}

DC_TEST("DataConfig.Core.Serialize.NonStringKeyMaps")
{
	FDcTestStructMaps Value;
	Value.MakeFixture();

	FDcPropertyDatum Datum(&Value);
	FDcJsonWriter Writer;

	FString ExpectStr = DcAutomationUtils::DcReindentStringLiteral(TEXT(R"(

		{
			"ColorKeyMap" : [
				{
					"$key" : "#FF0000FF",
					"$value" : "Red"
				},
				{
					"$key" : "#00FF00FF",
					"$value" : "Green"
				},
				{
					"$key" : "#0000FFFF",
					"$value" : "Blue"
				}
			],
			"EnumFlagsMap" : [
				{
					"$key" : [],
					"$value" : "None"
				},
				{
					"$key" : [
						"One",
						"Three"
					],
					"$value" : "One | Three"
				},
				{
					"$key" : [
						"Five"
					],
					"$value" : "Five"
				}
			]
		}

	)"));

	UTEST_OK("Serialize NonStringKeyMaps", DcAutomationUtils::SerializeInto(&Writer, Datum, [](FDcSerializeContext& Ctx){
		Ctx.Serializer->AddPredicatedHandler(
			FDcSerializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
			FDcSerializeDelegate::CreateStatic(DcExtra::HandlerColorSerialize)
		);
	}));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize NonStringKeyMaps", Writer.Sb.ToString(), ExpectStr);

	return true;
}

DC_TEST("DataConfig.Core.Serialize.EnumPropertyWithoutEnum")
{
	using namespace DcPropertyUtils;

	{
		//	this happens with BP enum field with stale enum reference
		uint8 Ch = 3;
		auto EnumProp = FDcPropertyBuilder::Enum(nullptr, FDcPropertyBuilder::Byte(TEXT("UnderlyingByte"))).LinkOnScope();

		FDcJsonWriter Writer;
		UTEST_OK("Serialize EnumPropertyWithoutEnum", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(EnumProp.Get(), &Ch)));
		UTEST_EQUAL("Serialize EnumPropertyWithoutEnum", Writer.Sb.ToString(), "3");
	}

	{
		uint8 Ch = 5;
		auto ByteProp = FDcPropertyBuilder::Byte(nullptr).LinkOnScope();
		
		FDcJsonWriter Writer;
		UTEST_OK("Serialize EnumPropertyWithoutEnum", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(ByteProp.Get(), &Ch)));
		UTEST_EQUAL("Serialize EnumPropertyWithoutEnum", Writer.Sb.ToString(), "5");
	}

	{
		uint8 Ch = (uint8)EDcTestEnum_UInt8::Max;
		auto ByteProp = FDcPropertyBuilder::Byte(StaticEnum<EDcTestEnum_UInt8>()).LinkOnScope();
		
		FDcJsonWriter Writer;
		UTEST_OK("Serialize EnumPropertyWithoutEnum", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(ByteProp.Get(), &Ch)));
		UTEST_EQUAL("Serialize EnumPropertyWithoutEnum", Writer.Sb.ToString(), "\"Max\"");

		Ch = 3;
		UTEST_DIAG("Serialize EnumPropertyWithoutEnum", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(ByteProp.Get(), &Ch)),
			DcDReadWrite, EnumValueInvalid);
	}

	return true;
}
