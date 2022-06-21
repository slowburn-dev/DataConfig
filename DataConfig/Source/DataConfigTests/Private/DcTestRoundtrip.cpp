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

void FDcTestRoundtripStruct1::MakeFixture()
{
	FieldPathField = DcPropertyUtils::FirstEffectiveProperty(FDcTestStruct1::StaticStruct()->PropertyLink);
}

void UDcTestRoundtrip2_Transient::MakeFixture()
{
	UDcTestDelegateClass1* Obj = NewObject<UDcTestDelegateClass1>();
	UObject* InterfaceObj = NewObject<UDcTestInterface1Beta>();

	DelegateField.BindDynamic(Obj, &UDcTestDelegateClass1::ReturnOne);
	MulticastField.AddDynamic(Obj, &UDcTestDelegateClass1::ReturnNone);
	SparseField.AddDynamic(Obj, &UDcTestDelegateClass1::ReturnNone);
	InterfaceField = InterfaceObj;
}

DC_TEST("DataConfig.Core.RoundTrip.JsonRoundtrip1_Default")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcJsonWriter JsonWriter;
	{
		UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupJsonSerializeHandlers(*Ctx.Serializer, EDcJsonSerializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	FString Json = JsonWriter.Sb.ToString();
	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcJsonReader Reader(Json);
		UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupJsonDeserializeHandlers(*Ctx.Deserializer, EDcJsonDeserializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}

DC_TEST("DataConfig.Core.RoundTrip.JsonRoundtrip2_StringSoftLazy")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcJsonWriter JsonWriter;
	{
		UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::SerializeInto(&JsonWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupJsonSerializeHandlers(*Ctx.Serializer, EDcJsonSerializeType::StringSoftLazy);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	FString Json = JsonWriter.Sb.ToString();
	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcJsonReader Reader(Json);
		UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupJsonDeserializeHandlers(*Ctx.Deserializer, EDcJsonDeserializeType::StringSoftLazy);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("Json SerDe Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}

DC_TEST("DataConfig.Core.RoundTrip.PropertyRoundtrip1")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	UDcTestRoundtrip1* Middle = NewObject<UDcTestRoundtrip1>();
	Middle->StructInlineSub.MakeFixtureEmpty();

	FDcPropertyDatum MiddleDatum(Middle);
	{
		FDcPropertyWriter Writer(MiddleDatum);
		UTEST_OK("Property SerDe Roundtrip 1", DcAutomationUtils::SerializeInto(&Writer, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupPropertyPipeSerializeHandlers(*Ctx.Serializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	Dest->StructInlineSub.MakeFixtureEmpty();
	FDcPropertyDatum DestDatum(Dest);
	{
		FDcPropertyReader Reader(MiddleDatum);
		UTEST_OK("Property SerDe Roundtrip 1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			DcSetupPropertyPipeDeserializeHandlers(*Ctx.Deserializer);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("Property SerDe Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}

DC_TEST("DataConfig.Core.RoundTrip.MsgPack_InMemory")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcMsgPackWriter MsgPackWriter;
	{
		UTEST_OK("MsgPack SerDe Roundtrip 1", DcAutomationUtils::SerializeInto(&MsgPackWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			verify(Ctx.Reader->SetConfig(FDcPropertyConfig::MakeNoExpandObject()).Ok());
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::InMemory);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	FDcMsgPackWriter::BufferType& Buffer = MsgPackWriter.GetMainBuffer();

	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcMsgPackReader Reader({Buffer.GetData(), Buffer.Num()});
		UTEST_OK("MsgPack SerDe Roundtrip 1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			verify(Ctx.Writer->SetConfig(FDcPropertyConfig::MakeNoExpandObject()).Ok());
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::InMemory);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("MsgPack SerDe Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}


DC_TEST("DataConfig.Core.RoundTrip.MsgPack_InMemory2")
{
	UDcTestRoundtrip2_Transient* Source = NewObject<UDcTestRoundtrip2_Transient>();
	Source->MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcMsgPackWriter MsgPackWriter;
	{
		UTEST_OK("MsgPack SerDe Roundtrip 2", DcAutomationUtils::SerializeInto(&MsgPackWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			verify(Ctx.Reader->SetConfig(FDcPropertyConfig::MakeNoExpandObject()).Ok());
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::InMemory);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	FDcMsgPackWriter::BufferType& Buffer = MsgPackWriter.GetMainBuffer();

	UDcTestRoundtrip2_Transient* Dest = NewObject<UDcTestRoundtrip2_Transient>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcMsgPackReader Reader({Buffer.GetData(), Buffer.Num()});
		UTEST_OK("MsgPack SerDe Roundtrip 2", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			verify(Ctx.Writer->SetConfig(FDcPropertyConfig::MakeNoExpandObject()).Ok());
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::InMemory);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("MsgPack SerDe Roundtrip 2", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}


DC_TEST("DataConfig.Core.RoundTrip.MsgPack_Persistent_Default")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcMsgPackWriter MsgPackWriter;
	{
		UTEST_OK("MsgPack Persistent SerDe Roundtrip 1", DcAutomationUtils::SerializeInto(&MsgPackWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	auto& Buffer = MsgPackWriter.GetMainBuffer();
	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcMsgPackReader Reader(FDcBlobViewData::From(Buffer));
		UTEST_OK("MsgPack Persistent SerDe Roundtrip 1", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::Default);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("MsgPack Persistent SerDe Roundtrip 1", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}


DC_TEST("DataConfig.Core.RoundTrip.MsgPack_Persistent_StringSoftLazy")
{
	UDcTestRoundtrip1* Source = NewObject<UDcTestRoundtrip1>();
	Source->StructPrimitives.MakeFixture();
	Source->StructEnumFlag.MakeFixture();
	Source->StructArrayDims.MakeFixture();
	Source->StructContainers.MakeFixtureFull();
	Source->StructOthers.MakeFixture();
	Source->StructInlineSub.MakeFixture();
	Source->StructObjectRefs.MakeFixture();
	Source->StructClassRefs.MakeFixture();
	FDcPropertyDatum SourceDatum(Source);

	FDcMsgPackWriter MsgPackWriter;
	{
		UTEST_OK("MsgPack Persistent SerDe Roundtrip 2", DcAutomationUtils::SerializeInto(&MsgPackWriter, SourceDatum,
		[](FDcSerializeContext& Ctx)
		{
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, EDcMsgPackSerializeType::StringSoftLazy);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	auto& Buffer = MsgPackWriter.GetMainBuffer();
	UDcTestRoundtrip1* Dest = NewObject<UDcTestRoundtrip1>();
	FDcPropertyDatum DestDatum(Dest);
	{

		FDcMsgPackReader Reader(FDcBlobViewData::From(Buffer));
		UTEST_OK("MsgPack Persistent SerDe Roundtrip 2", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx)
		{
			Ctx.Objects.Add(GetTransientPackage());
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, EDcMsgPackDeserializeType::StringSoftLazy);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));
	}

	UTEST_OK("MsgPack Persistent SerDe Roundtrip 2", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));
	return true;
}


