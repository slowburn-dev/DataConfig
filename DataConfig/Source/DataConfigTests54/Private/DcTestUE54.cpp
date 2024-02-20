#include "DcTestUE54.h"

#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"


DC_TEST("DataConfig.Core.Property.UE54.Optional.Blurb1")
{
	UTEST_OK("Optional Blurb1", []{

	using namespace DcPropertyUtils;

	auto OptionalStrProp = FDcPropertyBuilder::Optional(
		FDcPropertyBuilder::Str()
		).LinkOnScope();

	TOptional<FString> Source;

	{
		FDcPropertyWriter Writer(FDcPropertyDatum(OptionalStrProp.Get(), &Source));

		DC_TRY(Writer.WriteOptionalRoot());					// Optional Root
		DC_TRY(Writer.WriteString(TEXT("Some String")));	// Str
		DC_TRY(Writer.WriteOptionalEnd());					// Optional End

		check(*Source == TEXT("Some String"));


		FDcPropertyReader Reader(FDcPropertyDatum(OptionalStrProp.Get(), &Source));
		
		FString ReadStr;
		DC_TRY(Reader.ReadOptionalRoot());		// Optional Root
		DC_TRY(Reader.ReadString(&ReadStr));	// Str
		check(ReadStr == TEXT("Some String"));	
		DC_TRY(Reader.ReadOptionalEnd());		// Optional End
	}

	{
		FDcPropertyWriter Writer(FDcPropertyDatum(OptionalStrProp.Get(), &Source));

		DC_TRY(Writer.WriteOptionalRoot());	//	Optional Root
		DC_TRY(Writer.WriteNone());			//	None
		DC_TRY(Writer.WriteOptionalEnd());	//	Optional End
		
		check(!Source.IsSet());

		FDcPropertyReader Reader(FDcPropertyDatum(OptionalStrProp.Get(), &Source));

		DC_TRY(Reader.ReadOptionalRoot());	// Optional Root
		DC_TRY(Reader.ReadNone());			// None
		DC_TRY(Reader.ReadOptionalEnd());	// Optional End
	}

	return DcOk();

	}());

	return true;
}

DC_TEST("DataConfig.Core.Property.UE54.Optional.Blurb2")
{
    FString Str = TEXT(R"(

	    {
	    	"OptionalFloatField1" : 17.5,
	    	"OptionalFloatField2" : null,

	    	"OptionalStrField1" : "Alpha",
	    	"OptionalStrField2" : null,

	    	"OptionalStructField1" : {
	    		"StrField" : "Beta",
	    		"IntField" : 42
	    	},
	    	"OptionalStructField2" : null
	    }

    )");

	FDcTestOptional Expect;
	Expect.OptionalFloatField1 = 17.5f;
	Expect.OptionalStrField1 = TEXT("Alpha");
	Expect.OptionalStructField1.Emplace();
	Expect.OptionalStructField1->StrField = TEXT("Beta");
	Expect.OptionalStructField1->IntField = 42;

	{
		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);
        FDcJsonReader Reader(Str);

        UTEST_OK("Optional Blurb", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
        [](FDcDeserializeContext& Ctx) {}));

		FDcPropertyDatum ExpectDatum(&Expect);
		UTEST_OK("Optional Blurb", DcAutomationUtils::TestReadDatumEqual(ExpectDatum, DestDatum));
	}

	return true;
}


DC_TEST("DataConfig.Core.Property.UE54.Optional")
{
	FDcTestOptional Source;

	Source.OptionalFloatField1 = 17.5f;
	Source.OptionalStrField1 = TEXT("5.4 Added Optional Field");
	Source.OptionalStructField1.Emplace();
	Source.OptionalStructField1->StrField = TEXT("Inner Text");
	Source.OptionalStructField1->IntField = 42;

	FDcPropertyDatum SourceDatum(&Source);

	{
		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);

		UTEST_OK("TOptional roundtrip", DcPropertyPipeVisit(SourceDatum, DestDatum));
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

		UTEST_TRUE("TOptional roundtrip", Dest.OptionalFloatField1.IsSet());
		UTEST_TRUE("TOptional roundtrip", *Dest.OptionalFloatField1 == 17.5f);
		UTEST_TRUE("TOptional roundtrip", !Dest.OptionalFloatField2.IsSet());
		UTEST_TRUE("TOptional roundtrip", Dest.OptionalStrField1.IsSet());
		UTEST_TRUE("TOptional roundtrip", *Dest.OptionalStrField1 == *Source.OptionalStrField1);
		UTEST_TRUE("TOptional roundtrip", !Dest.OptionalStrField2.IsSet());

		UTEST_TRUE("TOptional roundtrip", Dest.OptionalStructField1.IsSet());
		UTEST_TRUE("TOptional roundtrip", Dest.OptionalStructField1->StrField == Source.OptionalStructField1->StrField);
		UTEST_TRUE("TOptional roundtrip", Dest.OptionalStructField1->IntField == Source.OptionalStructField1->IntField);
		UTEST_TRUE("TOptional roundtrip", !Dest.OptionalStructField2.IsSet());
	}

	{
		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);

		FDcPropertyWriter Writer(DestDatum);
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::SerializeInto(&Writer, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupPropertyPipeSerializeHandlers(*Ctx.Serializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_OK("TOptional roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	}

	{
		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);

		FDcPropertyReader Reader(SourceDatum);
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			DcSetupPropertyPipeDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_OK("TOptional roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	}

	{
		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);

		FDcJsonWriter JsonWriter;
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum,
		[](FDcSerializeContext&){}));

		FString Json = JsonWriter.Sb.ToString();

		FDcJsonReader JsonReader{Json};
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::DeserializeFrom(&JsonReader, DestDatum,
		[](FDcDeserializeContext& Ctx){}));

		UTEST_OK("TOptional roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	}

	{
		FDcMsgPackWriter::BufferType MsgPackBytes;

		FDcTestOptional Dest;
		FDcPropertyDatum DestDatum(&Dest);

		FDcMsgPackWriter MsgPackWriter;
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::SerializeInto(&MsgPackWriter, SourceDatum,
		[](FDcSerializeContext& Ctx){
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
		MsgPackBytes = MoveTemp(MsgPackWriter.GetMainBuffer());

		FDcMsgPackReader MsgPackReader({MsgPackBytes.GetData(), MsgPackBytes.Num()});
		UTEST_OK("TOptional roundtrip", DcAutomationUtils::DeserializeFrom(&MsgPackReader, DestDatum,
		[](FDcDeserializeContext& Ctx){
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_OK("TOptional roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	}

	return true;
}

DC_TEST("DataConfig.Core.Property.UE54.OptionalBuilder")
{
	using namespace DcPropertyUtils;

	{
		auto OptionalStrProp = FDcPropertyBuilder::Optional(
			FDcPropertyBuilder::Str()
			).LinkOnScope();

		TOptional<FString> Source{TEXT("Hello")};
		TOptional<FString> Dest;

		UTEST_OK("OptionalBuilder Roundtrip", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(OptionalStrProp.Get(), &Source),
			FDcPropertyDatum(OptionalStrProp.Get(), &Dest)
		));
	}

	{
		auto OptionalStructProp = FDcPropertyBuilder::Optional(
			FDcPropertyBuilder::Struct(FDcInnerStruct54::StaticStruct())
			).LinkOnScope();

		TOptional<FDcInnerStruct54> Source;
		Source.Emplace();
		Source->StrField = TEXT("Inner Text");
		Source->IntField = 42;
		TOptional<FDcInnerStruct54> Dest;

		UTEST_OK("OptionalBuilder Roundtrip", DcPropertyPipeVisitAndTestEqual(
			FDcPropertyDatum(OptionalStructProp.Get(), &Source),
			FDcPropertyDatum(OptionalStructProp.Get(), &Dest)
		));
	}

	return true;
}
