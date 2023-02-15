#include "DcTestProperty2.h"
#include "DcTestProperty4.h"
#include "DcTestSerDe.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"

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


#if WITH_EDITORONLY_DATA
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
#endif // WITH_EDITORONLY_DATA

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

	Expect.ObjField1 = FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests"), true);
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
			"StructSubClassField3" : "Function",
		}

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructSubClass1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructSubClass1 Expect;

	Expect.StructSubClassField1 = nullptr;
	Expect.StructSubClassField2 = UScriptStruct::StaticClass();
	Expect.StructSubClassField3 = UFunction::StaticClass();

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


DC_TEST("DataConfig.Core.Deserialize.NonStructClassRoots")
{
	FString ExpectStr = TEXT("These are my twisted words");

	uint64 ExpectU64Arr[2] = {TNumericLimits<uint64>::Min(), TNumericLimits<uint64>::Max()};

	TArray<FDcTestStructSimple> ExpectArr = {
		{TEXT("Foo"), TEXT("Bar")},
		{TEXT("Doo"), TEXT("Dar")},
		{TEXT("Goo"), TEXT("Gar")},
	};

	TSet<EDcTestEnum1> ExpectSet = {
		EDcTestEnum1::Foo,
		EDcTestEnum1::Bar,
		EDcTestEnum1::Tard,
	};

	TMap<FString, int> ExpectMap = {
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


	FString DestStr;
	uint64 DestU64Arr[2];
	TArray<FDcTestStructSimple> DestArr;
	TSet<EDcTestEnum1> DestSet;
	TMap<FString, int> DestMap;

	auto _CheckDeserializeEqual = [](FDcAutomationBase* Self, FString SrcStr, FDcPropertyDatum Dest, FDcPropertyDatum Expect) -> bool
	{
		FDcJsonReader Reader(SrcStr);
		if (!Self->TestOk("Deserialize NonStructClassRoots", DcAutomationUtils::DeserializeFrom(&Reader, Dest)))
			return false;

		if (!Self->TestOk("Deserialize NonStructClassRoots", DcAutomationUtils::TestReadDatumEqual(Dest, Expect)))
			return false;

		return true;
	};

	UTEST_TRUE("Deserialize NonStructClassRoots",  _CheckDeserializeEqual(
		this,
		TEXT(R"(
			"These are my twisted words"
		)"),
		FDcPropertyDatum(StrProp.Get(), &DestStr),
		FDcPropertyDatum(StrProp.Get(), &ExpectStr)
		));

	UTEST_TRUE("Deserialize NonStructClassRoots",  _CheckDeserializeEqual(
		this,
		TEXT(R"(
			[
				0,
				18446744073709551615
			]
		)"),
		FDcPropertyDatum(U64ArrProp.Get(), &DestU64Arr),
		FDcPropertyDatum(U64ArrProp.Get(), &ExpectU64Arr)
		));

	UTEST_TRUE("Deserialize NonStructClassRoots",  _CheckDeserializeEqual(
		this,
		TEXT(R"(
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
		)"),
		FDcPropertyDatum(ArrProp.Get(), &DestArr),
		FDcPropertyDatum(ArrProp.Get(), &ExpectArr)
		));

	UTEST_TRUE("Deserialize NonStructClassRoots",  _CheckDeserializeEqual(
		this,
		TEXT(R"(
			[
				"Foo",
				"Bar",
				"Tard"
			]		
		)"),
		FDcPropertyDatum(SetProp.Get(), &DestSet),
		FDcPropertyDatum(SetProp.Get(), &ExpectSet)
		));


	UTEST_TRUE("Deserialize NonStructClassRoots",  _CheckDeserializeEqual(
		this,
		TEXT(R"(
			{
				"One" : 1,
				"Two" : 2,
				"Three" : 3
			}
		)"),
		FDcPropertyDatum(MapProp.Get(), &DestMap),
		FDcPropertyDatum(MapProp.Get(), &ExpectMap)
		));

	return true;
}

#if WITH_EDITORONLY_DATA
DC_TEST("DataConfig.Core.Deserialize.NonStringKeyMaps")
{
	FString Str = TEXT(R"(

		{
			"ColorKeyMap" : {
				"#FF0000FF" : "Red",
				"#00FF00FF" : "Green",
				"#0000FFFF" : "Blue"
			},
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

	)");
	FDcJsonReader Reader(Str);

	FDcTestStructMaps Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FDcTestStructMaps Expect;
	Expect.MakeFixture();

	FDcPropertyDatum ExpectDatum(&Expect);

	UTEST_OK("Deserialize NonStringKeyMaps", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum, [](FDcDeserializeContext& Ctx){
		Ctx.Deserializer->AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
			FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
		);
	}));
	UTEST_OK("Deserialize NonStringKeyMaps", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}
#endif // WITH_EDITORONLY_DATA

DC_TEST("DataConfig.Core.Deserialize.EnumPropertyWithoutEnum")
{
	using namespace DcPropertyUtils;

	{
		//	this happens with BP enum field with stale enum reference
		uint8 Ch;
		auto EnumProp = FDcPropertyBuilder::Enum(nullptr, FDcPropertyBuilder::Byte(TEXT("UnderlyingByte"))).LinkOnScope();

		FDcJsonReader Reader(TEXT("3"));
		UTEST_OK("Deserialize EnumPropertyWithoutEnum", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(EnumProp.Get(), &Ch)));
		UTEST_EQUAL("Deserialize EnumPropertyWithoutEnum", Ch, 3);
	}

	{
		uint8 Ch;
		auto ByteProp = FDcPropertyBuilder::Byte(nullptr).LinkOnScope();

		FDcJsonReader Reader(TEXT("5"));
		UTEST_OK("Deserialize EnumPropertyWithoutEnum", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(ByteProp.Get(), &Ch)));
		UTEST_EQUAL("Deserialize EnumPropertyWithoutEnum", Ch, 5);
	}

	{
		uint8 Ch;
		auto ByteProp = FDcPropertyBuilder::Byte(StaticEnum<EDcTestEnum_UInt8>()).LinkOnScope();

		FDcJsonReader Reader(TEXT("\"Max\""));
		UTEST_OK("Deserialize EnumPropertyWithoutEnum", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(ByteProp.Get(), &Ch)));
		UTEST_EQUAL("Deserialize EnumPropertyWithoutEnum", Ch, (uint8)EDcTestEnum_UInt8::Max);

		FDcJsonReader Reader2(TEXT("\"Foo\""));
		UTEST_DIAG("Deserialize EnumPropertyWithoutEnum", DcAutomationUtils::DeserializeFrom(&Reader2, FDcPropertyDatum(ByteProp.Get(), &Ch)),
			DcDReadWrite, EnumNameNotFound);
	}

	return true;
}

FDcResult _NaiveReadObjReference(FDcDeserializeContext& Ctx, FObjectPropertyBase* /*ObjectProperty*/, UObject*& OutObject)
{
	//	note it doesn't respect ObjectProperty class
	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));
	return DcSerDeUtils::TryStaticLocateObject(
		UObject::StaticClass(),
		*Value,
		OutObject);
};

DC_TEST("DataConfig.Core.Deserialize.DiagObjectClassMismatch")
{
	FDcTestObjectRefs2 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	{
		FString Str = TEXT(R"(

			{
				"TestClassObjectField" : "'/Script/DataConfigTests'",
			}

		)");

		FDcJsonReader Reader(Str);
		UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum, [](FDcDeserializeContext& Ctx) {

			Ctx.Deserializer->AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));
			Ctx.Deserializer->AddDirectHandler(FObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx){
				return DcDeserializeObjectReference<_NaiveReadObjReference>(Ctx);
			}));

		}, DcAutomationUtils::EDefaultSetupType::SetupNothing), DcDSerDe, ClassLhsIsNotChildOfRhs);
	}

	{
		FString Str = TEXT(R"(

			{
				"TestClassSoftObjectField" : "'/Script/DataConfigTests'",
			}

		)");

		FDcJsonReader Reader(Str);
		UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum, [](FDcDeserializeContext& Ctx) {

			Ctx.Deserializer->AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));
			Ctx.Deserializer->AddDirectHandler(FSoftObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx){
				return DcDeserializeSoftObjectReference<_NaiveReadObjReference>(Ctx);
			}));

		}, DcAutomationUtils::EDefaultSetupType::SetupNothing), DcDSerDe, ClassLhsIsNotChildOfRhs);
	}

	{
		FString Str = TEXT(R"(

			{
				"TestClassWeakObjectField" : "'/Script/DataConfigTests'",
			}

		)");

		FDcJsonReader Reader(Str);
		UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum, [](FDcDeserializeContext& Ctx) {

			Ctx.Deserializer->AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));
			Ctx.Deserializer->AddDirectHandler(FWeakObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx){
				return DcDeserializeWeakObjectReference<_NaiveReadObjReference>(Ctx);
			}));

		}, DcAutomationUtils::EDefaultSetupType::SetupNothing), DcDSerDe, ClassLhsIsNotChildOfRhs);
	}

	{
		FString Str = TEXT(R"(

			{
				"TestClassLazyObjectField" : "'/Script/DataConfigTests'",
			}

		)");

		FDcJsonReader Reader(Str);
		UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum, [](FDcDeserializeContext& Ctx) {

			Ctx.Deserializer->AddDirectHandler(UScriptStruct::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToStructDeserialize));
			Ctx.Deserializer->AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx){
				return DcDeserializeLazyObjectReference<_NaiveReadObjReference>(Ctx);
			}));

		}, DcAutomationUtils::EDefaultSetupType::SetupNothing), DcDSerDe, ClassLhsIsNotChildOfRhs);
	}


	{
		FString Str = TEXT(R"(

			{
				"InlineShapeObjectField" : {
					"$type": "DcTestClass1",
					"IntField": 253
				}
			}

		)");

		FDcJsonReader Reader(Str);
		UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum), DcDSerDe, ClassLhsIsNotChildOfRhs);
	}

	return true;
};

