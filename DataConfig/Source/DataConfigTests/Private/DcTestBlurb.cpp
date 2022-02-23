#include "DcTestBlurb.h"
#include "DcTestSerDe.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/Deserialize/DcDeserializeTypes.h"
#include "DataConfig/Diagnostic/DcDiagnosticCommon.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Serialize/DcSerializerSetup.h"
#include "DataConfig/Serialize/DcSerializeTypes.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"

/// short test code for example and showcases
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

		{
			//	create and setup a deserializer
			FDcDeserializer Deserializer;
			DcSetupJsonDeserializeHandlers(Deserializer);
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(DcExtra::PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(DcExtra::HandlerColorDeserialize)
			);

			//	prepare deserialize context
			FDcPropertyDatum Datum(&Dest);
			FDcJsonReader Reader(Str);
			FDcPropertyWriter Writer(Datum);

			FDcDeserializeContext Ctx;
			Ctx.Reader = &Reader;
			Ctx.Writer = &Writer;
			Ctx.Deserializer = &Deserializer;
			DC_TRY(Ctx.Prepare());

			//	kick off deserialization
			DC_TRY(Deserializer.Deserialize(Ctx));
		}

		//	validate results
		check(Dest.StrField == TEXT("Lorem ipsum dolor sit amet"));
		check(Dest.EnumField == EDcTestExampleEnum::Bar);
		check(Dest.Colors[0] == FColor::Red);
		check(Dest.Colors[1] == FColor::Green);
		check(Dest.Colors[2] == FColor::Blue);

		//	then serialize to MsgPack
		{
			FDcSerializer Serializer;
			DcSetupMsgPackSerializeHandlers(Serializer);

			FDcPropertyDatum Datum(&Dest);
			FDcPropertyReader Reader(Datum);
			FDcMsgPackWriter Writer;

			//	prepare serialize context
			FDcSerializeContext Ctx;
			Ctx.Reader = &Reader;
			Ctx.Writer = &Writer;
			Ctx.Serializer = &Serializer;
			DC_TRY(Ctx.Prepare());

			//	kick off serialization
			DC_TRY(Serializer.Serialize(Ctx));

			auto& Buffer = Writer.GetMainBuffer();
			//	starts withMsgPack FIXMAP(3) header
			check(Buffer[0] == 0x83);	
		}

		return DcOk();
	}());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.Dump")
{
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::NoLogging);

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

		FDcPropertyReader Reader{FDcPropertyDatum(&SimpleStruct)};

		DC_TRY(Reader.ReadStructRoot()); // `FDcTestExampleSimple` Struct Root

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

		DC_TRY(Reader.ReadStructEnd());	// `FDcTestExampleSimple` Struct Root

		return DcOk();
	}());

	UTEST_OK("Blurb Writer", [&]{

		FDcPropertyWriter Writer{FDcPropertyDatum(&SimpleStruct)};

		DC_TRY(Writer.WriteStructRoot()); // `FDcTestExampleSimple` Struct Root

		DC_TRY(Writer.WriteName(TEXT("StrField")));      // 'StrField' as FName
		DC_TRY(Writer.WriteString(TEXT("Alt Str")));     // "Foo STr"


		DC_TRY(Writer.WriteName(TEXT("IntField")));      // 'IntField' as FName
		DC_TRY(Writer.WriteInt32(233));                  // 233

		DC_TRY(Writer.WriteStructEnd());  // `FDcTestExampleSimple` Struct Root

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
	UTEST_OK("Blurb JSONReader", [&]{

		FString Str = TEXT(R"(
			{
				"Str" : "Fooo",
				"Number" : 1.875,
				"Bool" : true
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
	FLogScopedCategoryAndVerbosityOverride LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::NoLogging);

	UTEST_OK("Blurb Uninitialized", [&]{

		FString Str = TEXT(R"(
			{
				// pass
			} 
		)");
		FDcJsonReader Reader(Str);

		FDcTestExampleSimple Dest;
		FDcPropertyDatum DestDatum(&Dest);

		DC_TRY(DcAutomationUtils::DeserializeFrom(&Reader, DestDatum));

		check(Dest.StrField.IsEmpty());
		//	but Dest.IntField contains uninitialized value
		DcAutomationUtils::DumpToLog(DestDatum);

		return DcOk();

	}());

	return true;
};

DC_TEST("DataConfig.Core.Blurb.PropertyHandlers")
{
	 UTEST_OK("Blurb Uninitialized", [&]{

		FDcTestExampleSimple From;
		From.StrField = TEXT("Foo");
		From.IntField = 123;
		FDcTestExampleSimple To;

		//	these two following blocks are equivalent
		{
			To  = FDcTestExampleSimple();
			FDcPropertyDatum FromDatum(&From);
			FDcPropertyDatum ToDatum(&To);

			FDcPropertyReader Reader(FromDatum);
			FDcPropertyWriter Writer(ToDatum);
			FDcPipeVisitor RoundtripVisit(&Reader, &Writer);

			DC_TRY(RoundtripVisit.PipeVisit());
			DC_TRY(DcAutomationUtils::TestReadDatumEqual(FromDatum, ToDatum));
		}

		{
			To  = FDcTestExampleSimple();
			FDcPropertyDatum FromDatum(&From);
			FDcPropertyDatum ToDatum(&To);

			FDcDeserializer Deserializer;
			DcSetupPropertyPipeDeserializeHandlers(Deserializer);

			FDcPropertyReader Reader(FromDatum);
			FDcPropertyWriter Writer(ToDatum);

			FDcDeserializeContext Ctx;
			Ctx.Reader = &Reader;
			Ctx.Writer = &Writer;
			Ctx.Deserializer = &Deserializer;
			DC_TRY(Ctx.Prepare());

			DC_TRY(Deserializer.Deserialize(Ctx));
			DC_TRY(DcAutomationUtils::TestReadDatumEqual(FromDatum, ToDatum));
		}

		return DcOk();

	}());

	return true;
};


DC_TEST("DataConfig.Core.Blurb.JSONWriter")
{
	UTEST_OK("Blurb JSONWriter", [&]{

		FDcJsonWriter Writer;

		DC_TRY(Writer.WriteMapRoot());

			DC_TRY(Writer.WriteString(TEXT("Str")));
			DC_TRY(Writer.WriteString(TEXT("Fooo")));

			DC_TRY(Writer.WriteString(TEXT("Number")));
			DC_TRY(Writer.WriteFloat(1.875f));

			DC_TRY(Writer.WriteString(TEXT("Bool")));
			DC_TRY(Writer.WriteBool(true));

		DC_TRY(Writer.WriteMapEnd());
		Writer.Sb << TCHAR('\n');

		FString Str = TEXT(R"(
			{
				"Str" : "Fooo",
				"Number" : 1.875,
				"Bool" : true
			}
		)");

		//	validate results
		check(DcReindentStringLiteral(Str) == Writer.Sb.ToString());
		return DcOk();

	}());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.StringSoftLazy")
{
	UTEST_OK("Blurb StringSoftLazy", [&]{

		FDcTestStructRefs1 Source{};
		UObject* TestsObject = StaticFindObject(UObject::StaticClass(), nullptr, TEXT("/Script/DataConfigTests"));

		Source.SoftField1 = TestsObject;
		Source.LazyField1 = TestsObject;

		FDcJsonWriter Writer;
		DC_TRY(DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Source),
		[](FDcSerializeContext& Ctx) {
			DcSetupJsonSerializeHandlers(*Ctx.Serializer, EDcJsonSerializeType::StringSoftLazy);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		/*
		FLogScopedCategoryAndVerbosityOverride  LogOverride(TEXT("LogDataConfigCore"), ELogVerbosity::Display);
		FString Str = Writer.Sb.ToString();
		UE_LOG(LogDataConfigCore, Display, TEXT("SoftLazy: %s"), *Str);
		*/

		return DcOk();

	}());

	return true;

}


DC_TEST("DataConfig.Core.Blurb.MsgPackBlobExt")
{
	TArray<uint8> Arr = {1,2,3,4,5};
	UTEST_OK("Blurb MsgPackBlobExt", [&]{

		FDcMsgPackWriter Writer;
		DC_TRY(Writer.WriteBlob(FDcBlobViewData::From(Arr)));
		auto& Buf = Writer.GetMainBuffer();

		FDcMsgPackReader Reader(FDcBlobViewData::From(Buf));
		FDcBlobViewData Blob;
		DC_TRY(Reader.ReadBlob(&Blob));

		check(Blob.Num == 5);
		check(FPlatformMemory::Memcmp(Arr.GetData(), Blob.DataPtr, Blob.Num) == 0);

		return DcOk();
	}());

	UTEST_OK("Blurb MsgPackBlobExt", [&]{

		FDcMsgPackWriter Writer;
		DC_TRY(Writer.WriteFixExt2(1, {2, 3}));
		auto& Buf = Writer.GetMainBuffer();

		FDcMsgPackReader Reader(FDcBlobViewData::From(Buf));
		uint8 Type;
		FDcBytes2 Bytes;
		DC_TRY(Reader.ReadFixExt2(&Type, &Bytes));

		check(Type == 1);
		check(Bytes.Data[0] == 2);
		check(Bytes.Data[1] == 3);

		return DcOk();
	}());

	return true;

}
