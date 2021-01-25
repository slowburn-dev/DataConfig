#include "DcTestBlurb.h"
#include "DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"

// short test code for example and showcases

DC_TEST("DataConfig.Core.Blurb.Frontpage")
{
	UTEST_OK("Blurb Frontpage", []{

		FString Str = TEXT(R"(
			{
				"StrField" : "Lorem ipsum dolor sit amet",
				"EnumField" : "Bar",
				"Colors" : [
					"#FF0000FF", "#00FF00FF", "#0000FFFF"
				]
			}
		)");

		FDcTestExampleStruct Dest;

		//	create and setup a deserializer
		FDcDeserializer Deserializer;
		DcSetupJsonDeserializeHandlers(Deserializer);
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
			FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
		);

		//	prepare context for this run
		FDcPropertyDatum Datum(FDcTestExampleStruct::StaticStruct(), &Dest);
		FDcJsonReader Reader(Str);
		FDcPropertyWriter Writer(Datum);

		FDcDeserializeContext Ctx;
		Ctx.Reader = &Reader;
		Ctx.Writer = &Writer;
		Ctx.Deserializer = &Deserializer;
		Ctx.Properties.Push(Datum.Property);
		DC_TRY(Ctx.Prepare());

		//	kick off deserialization
		DC_TRY(Deserializer.Deserialize(Ctx));

		//	validate results
		check(Dest.StrField == TEXT("Lorem ipsum dolor sit amet"));
		check(Dest.EnumField == EDcTestExampleEnum::Bar);
		check(Dest.Colors[0] == FColor::Red);
		check(Dest.Colors[1] == FColor::Green);
		check(Dest.Colors[2] == FColor::Blue);

		return DcOk();
	}());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.Dump")
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(FName(TEXT("LogDataConfigCore")), ELogVerbosity::NoLogging);

	FVector Vec(1.0f, 2.0f, 3.0f);
	FDcPropertyDatum VecDatum(TBaseStructure<FVector>::Get(), &Vec);

	DcAutomationUtils::DumpToLog(VecDatum);

	return true;
}

DC_TEST("DataConfig.Core.Blurb.ReadWrite")
{
	FDcTestExampleSimple SimpleStruct;
	SimpleStruct.StrField = TEXT("Foo Str");
	SimpleStruct.IntField = 253;

	UTEST_OK("Blurb Reader", [&]{

		FDcPropertyReader Reader(FDcPropertyDatum(FDcTestExampleSimple::StaticStruct(), &SimpleStruct));

		FDcStructStat Struct;	// `FDcTestExampleSimple` Struct Root
		DC_TRY(Reader.ReadStructRoot(&Struct));
		check(Struct.Name == TEXT("DcTestExampleSimple"));

			FName FieldName;
			DC_TRY(Reader.ReadName(&FieldName));	// 'StrField' as FName
			check(FieldName == TEXT("StrField"));

			FString StrValue;
			DC_TRY(Reader.ReadString(&StrValue));	// "Foo STr"
			check(StrValue == TEXT("Foo Str"));

			DC_TRY(Reader.ReadName(&FieldName));	// 'IntField' as FName
			check(FieldName == TEXT("IntField"));

			int IntValue;
			DC_TRY(Reader.ReadInt32(&IntValue));	// 253
			check(IntValue == 253);

		DC_TRY(Reader.ReadStructEnd(&Struct));	// `FDcTestExampleSimple` Struct Root
		check(Struct.Name == TEXT("DcTestExampleSimple"));

		return DcOk();
	}());


	return true;
}
