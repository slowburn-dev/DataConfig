#include "DcTestRoundtrip.h"
#include "DataConfig/Serialize/DcSerializer.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
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

#if ENGINE_MAJOR_VERSION == 5

DC_TEST("DataConfig.Core.RoundTrip.TObjectPtr")
{
	using namespace DcPropertyUtils;

	{
		auto Prop = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::ObjectPtr(UObject::StaticClass())
		).LinkOnScope();

		UObject* TestsObject = StaticFindObject(UObject::StaticClass(), nullptr, TEXT("/Script/DataConfigTests"));
		UObject* NewObj = NewObject<UDcTestClass1>();

		TArray<TObjectPtr<UObject>> Source;

		Source.Add(TestsObject);
		Source.Add(NewObj);
		Source.Add(nullptr);


		FDcPropertyDatum SourceDatum(Prop.Get(), &Source);

		{
			//	Property -> Json -> Property
			TArray<TObjectPtr<UObject>> Dest;
			FDcPropertyDatum DestDatum(Prop.Get(), &Dest);
			FDcCondensedJsonWriter JsonWriter;
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum, [](FDcSerializeContext& Ctx)
			{
				return;
			}));
			FDcJsonReader JsonReader(JsonWriter.Sb.ToString());
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::DeserializeFrom(&JsonReader, DestDatum, [](FDcDeserializeContext& Ctx)
			{
				return;
			}));

			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
		}

		{
			//	Property Roundtrip
			TArray<TObjectPtr<UObject>> Middle;
			TArray<TObjectPtr<UObject>> Dest;
			FDcPropertyDatum MiddleDatum(Prop.Get(), &Middle);
			FDcPropertyDatum DestDatum(Prop.Get(), &Dest);

			FDcPropertyWriter Writer(MiddleDatum);
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::SerializeInto(&Writer, SourceDatum,
			[](FDcSerializeContext& Ctx)
			{
				DcSetupPropertyPipeSerializeHandlers(*Ctx.Serializer);
				return;	
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

			FDcPropertyReader Reader(MiddleDatum);
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
			[](FDcDeserializeContext& Ctx)
			{
				DcSetupPropertyPipeDeserializeHandlers(*Ctx.Deserializer);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
		}
	}

	{
		auto Prop = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::ClassPtr(UObject::StaticClass())
		).LinkOnScope();

		TArray<TObjectPtr<UClass>> Source;

		Source.Add(UDcTestClass1::StaticClass());
		Source.Add(UDcBaseShape::StaticClass());
		Source.Add(nullptr);

		FDcPropertyDatum SourceDatum(Prop.Get(), &Source);

		{
			//	Property -> Json -> Property
			TArray<TObjectPtr<UClass>> Dest;
			FDcPropertyDatum DestDatum(Prop.Get(), &Dest);
			FDcCondensedJsonWriter JsonWriter;
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum, [](FDcSerializeContext& Ctx)
			{
				return;
			}));
			FDcJsonReader JsonReader(JsonWriter.Sb.ToString());
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::DeserializeFrom(&JsonReader, DestDatum, [](FDcDeserializeContext& Ctx)
			{
				Ctx.Objects.Add(GetTransientPackage());
				return;
			}));

			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
		}

		{
			//	Property Roundtrip
			TArray<TObjectPtr<UClass>> Middle;
			TArray<TObjectPtr<UClass>> Dest;
			FDcPropertyDatum MiddleDatum(Prop.Get(), &Middle);
			FDcPropertyDatum DestDatum(Prop.Get(), &Dest);

			FDcPropertyWriter Writer(MiddleDatum);
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::SerializeInto(&Writer, SourceDatum,
			[](FDcSerializeContext& Ctx)
			{
				DcSetupPropertyPipeSerializeHandlers(*Ctx.Serializer);
				return;	
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

			FDcPropertyReader Reader(MiddleDatum);
			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
			[](FDcDeserializeContext& Ctx)
			{
				DcSetupPropertyPipeDeserializeHandlers(*Ctx.Deserializer);
			}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

			UTEST_OK("Roundtrip TObjectPtr", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
		}
	}

	return true;
}

#endif //ENGINE_MAJOR_VERSION == 5
