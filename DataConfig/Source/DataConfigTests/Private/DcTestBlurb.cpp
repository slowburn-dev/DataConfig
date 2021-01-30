#include "DcTestBlurb.h"
#include "DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Json/DcJsonReader.h"

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

	UTEST_OK("Blurb Writer", [&]{

		FDcPropertyWriter Writer(FDcPropertyDatum(FDcTestExampleSimple::StaticStruct(), &SimpleStruct));

		DC_TRY(Writer.WriteStructRoot(FDcStructStat{})); // `FDcTestExampleSimple` Struct Root

		DC_TRY(Writer.WriteName(TEXT("StrField")));      // 'StrField' as FName
		DC_TRY(Writer.WriteString(TEXT("Alt Str")));     // "Foo STr"


		DC_TRY(Writer.WriteName(TEXT("IntField")));      // 'IntField' as FName
		DC_TRY(Writer.WriteInt32(233));                  // 233

		DC_TRY(Writer.WriteStructEnd(FDcStructStat{}));  // `FDcTestExampleSimple` Struct Root

		check(SimpleStruct.StrField == TEXT("Alt Str"));
		check(SimpleStruct.IntField == 233);

		return DcOk();
	}());


	return true;
}

namespace DcBlurbDetails {

	FDcResult Succeed() {
		// succeed
		return DcOk();
	}

	FDcResult Fail() {
		// fail !
		return DC_FAIL(DcDCommon, Unexpected1) << "Custom Message";
	}

} // namespace DcBlurbDetails


DC_TEST("DataConfig.Core.Blurb.Result")
{
	UTEST_OK("Blurb Result", DcBlurbDetails::Succeed());
	UTEST_DIAG("Blurb Result", DcBlurbDetails::Fail(), DcDCommon, Unexpected1);

	return true;
}

DC_TEST("DataConfig.Core.Blurb.JSONReader")
{
	UTEST_OK("Blurb Writer", [&]{

		FString Str = TEXT(R"(
			{
				"Str":    "Fooo",
				"Number": 1.875,
				"Bool":   true
			} 
		)");

		FDcJsonReader Reader(Str);

		//	calling read methods
		FString KeyStr;
		FString GotStr;
		double GotNumber;
		bool GotBool;

		DC_TRY(Reader.ReadMapRoot());

			DC_TRY(Reader.ReadString(&KeyStr));
			DC_TRY(Reader.ReadString(&GotStr));

			DC_TRY(Reader.ReadString(&KeyStr));
			DC_TRY(Reader.ReadDouble(&GotNumber));

			DC_TRY(Reader.ReadString(&KeyStr));
			DC_TRY(Reader.ReadBool(&GotBool));

		DC_TRY(Reader.ReadMapEnd());

		//	validate results
		check(GotStr == TEXT("Fooo"));
		check(GotNumber == 1.875);
		check(GotBool == true);

		return DcOk();

	}());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.Uninitialized")
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(FName(TEXT("LogDataConfigCore")), ELogVerbosity::NoLogging);

	 UTEST_OK("Blurb Uninitialized", [&]{

		FString Str = TEXT(R"(
			{
				// pass
			} 
		)");
		FDcJsonReader Reader(Str);

		FDcTestExampleSimple Dest;
		FDcPropertyDatum DestDatum(FDcTestExampleSimple::StaticStruct(), &Dest);

		DC_TRY(DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum));

		check(Dest.StrField.IsEmpty());
		//	but Dest.IntField contains uninitialized value
		DcAutomationUtils::DumpToLog(DestDatum);

		return DcOk();

	}());

	return true;
};

