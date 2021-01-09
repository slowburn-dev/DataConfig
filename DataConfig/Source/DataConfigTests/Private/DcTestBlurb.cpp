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
};

