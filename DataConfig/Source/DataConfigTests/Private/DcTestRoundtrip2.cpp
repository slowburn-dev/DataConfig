#include "DcTestRoundtrip.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"

DC_TEST("DataConfig.Core.RoundTrip.Property_Json_MsgPack_Json_Property")
{
	// Property -> Json -> MsgPack -> Json -> Property
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructContainers.MakeFixtureNoStructMap();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	//	Serialize Property -> Json 
	FDcCondensedJsonWriter JsonWriter;
	{
		UTEST_OK("Property Json MsgPack Roundtrip 1", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupJsonSerializeHandlers(*Ctx.Serializer, EDcJsonSerializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	FDcMsgPackWriter MsgPackWriter;
	//	Pipe Json -> MsgPack
	{
		FString Json = JsonWriter.Sb.ToString();
		FDcJsonReader JsonReader(Json);
		FDcPipeVisitor PipeVisitor(&JsonReader, &MsgPackWriter);
		UTEST_OK("Property Json MsgPack Roundtrip 1", PipeVisitor.PipeVisit());
	}

	//	Pipe MsgPack -> Json
	FDcJsonWriter JsonWriter2;
	{
		FDcMsgPackReader MsgPackReader(FDcBlobViewData::From(MsgPackWriter.GetMainBuffer()));
		FDcPipeVisitor PipeVisitor(&MsgPackReader, &JsonWriter2);
		UTEST_OK("Property Json MsgPack Roundtrip 1", PipeVisitor.PipeVisit());
	}

	//	Deserialize Json -> Property
	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{
		FString Json = JsonWriter2.Sb.ToString();
		FDcJsonReader JsonReader(Json);
		
		UTEST_OK("Property Json MsgPack Roundtrip 1", DcAutomationUtils::DeserializeFrom(&JsonReader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupJsonDeserializeHandlers(*Ctx.Deserializer, EDcJsonDeserializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("Property Json MsgPack Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}

