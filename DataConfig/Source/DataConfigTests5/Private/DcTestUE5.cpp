#include "DcTestUE5.h"

#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"

DC_TEST("DataConfig.Core.Property.UE5.TObjectPtr")
{
	{
		FDcTest5ObjectPtrAlpha Source;

		Source.ObjPtrField1 = FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests5"), true);
		Source.ObjPtrField2 = nullptr;

		FDcTest5ObjectPtrAlpha Dest;

		FDcPropertyDatum SourceDatum(&Source);
		FDcPropertyDatum DestDatum(&Dest);

		UTEST_OK("TObjectPtr roundtrip", DcPropertyPipeVisit(SourceDatum, DestDatum));
		UTEST_OK("TObjectPtr roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

		UTEST_TRUE("TObjectPtr roundtrip", Dest.ObjPtrField1 == FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests5"), true));
		UTEST_TRUE("TObjectPtr roundtrip", !Dest.ObjPtrField2);
	}

	{
		FDcTest5ObjectPtrAlpha Source;

		Source.ObjPtrField1 = FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests5"), true);
		Source.ObjPtrField2 = nullptr;

		FDcTest5ObjectPtrAlpha Dest;

		FDcPropertyReader Reader{FDcPropertyDatum{&Source}};
		FDcPropertyWriter Writer{FDcPropertyDatum{&Dest}};

		FName FieldName;
		TObjectPtr<UObject> ObjPtr;

		UTEST_OK("TObjectPtr", Reader.ReadStructRoot());

			UTEST_OK("TObjectPtr", Reader.ReadName(&FieldName));
			UTEST_TRUE("TObjectPtr", FieldName == TEXT("ObjPtrField1"));
			UTEST_OK("TObjectPtr", Reader.ReadClassRoot());
			UTEST_OK("TObjectPtr", Reader.ReadTObjectPtr<UObject>(&ObjPtr));
			UTEST_TRUE("TObjectPtr", ObjPtr == FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests5"), true));
			UTEST_OK("TObjectPtr", Reader.ReadClassEnd());

			UTEST_OK("TObjectPtr", Reader.ReadName(&FieldName));
			UTEST_TRUE("TObjectPtr", FieldName == TEXT("ObjPtrField2"));
			UTEST_OK("TObjectPtr", Reader.ReadClassRoot());
			UTEST_OK("TObjectPtr", Reader.ReadNil());
			UTEST_OK("TObjectPtr", Reader.ReadClassEnd());

		UTEST_OK("TObjectPtr", Reader.ReadStructEnd());

		UTEST_OK("TObjectPtr", Writer.WriteStructRoot());

			UTEST_OK("TObjectPtr", Writer.WriteName(TEXT("ObjPtrField1")));
			UTEST_OK("TObjectPtr", Writer.WriteClassRoot());
			UTEST_OK("TObjectPtr", Writer.WriteTObjectPtr(ObjPtr));
			UTEST_OK("TObjectPtr", Writer.WriteClassEnd());

		UTEST_OK("TObjectPtr", Writer.WriteStructEnd());
		UTEST_TRUE("TObjectPtr", Dest.ObjPtrField1 == FindObject<UPackage>(nullptr, TEXT("/Script/DataConfigTests5"), true));
	}

    return true;
}

DC_TEST("DataConfig.Core.Property.UE5.LargeWorldCoordinates")
{
	FDcTestLWC Source;

    Source.VectorField = {1.5f, 1.75f, 1.875f};
    Source.Vector3fField = {1.5f, 1.75f, 1.875f};
    Source.Vector3dField = {1.5, 1.75, 1.875};

	FDcTestLWC Dest;

	FDcPropertyDatum SourceDatum(&Source);
	FDcPropertyDatum DestDatum(&Dest);

	UTEST_OK("LargeWorldCoordinates roundtrip", DcPropertyPipeVisit(SourceDatum, DestDatum));
	UTEST_OK("LargeWorldCoordinates roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, DestDatum));

	UTEST_TRUE("LargeWorldCoordinates roundtrip", Dest.VectorField.X == 1.5f);
	UTEST_TRUE("LargeWorldCoordinates roundtrip", Dest.VectorField.Y == 1.75f);
	UTEST_TRUE("LargeWorldCoordinates roundtrip", Dest.VectorField.Z == 1.875f);

	return true;
}

DC_TEST("DataConfig.Core.RoundTrip.UE5.TObjectPtr")
{
	using namespace DcPropertyUtils;

	{
		auto Prop = FDcPropertyBuilder::Array(
			FDcPropertyBuilder::ObjectPtr(UObject::StaticClass())
		).LinkOnScope();

		UObject* TestsObject = StaticFindObject(UObject::StaticClass(), nullptr, TEXT("/Script/DataConfigTests"));
		UObject* NewObj = NewObject<UDcTest5ClassBase>();

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

		Source.Add(UDcTest5ClassBase::StaticClass());
		Source.Add(UDcTest5ClassDerived::StaticClass());
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

