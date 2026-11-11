#include "DcTestUEBefore59.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Deserialize/Handlers/Common/DcCommonDeserializers.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/MsgPack/DcMsgPackReader.h"
#include "DataConfig/MsgPack/DcMsgPackWriter.h"
#include "DataConfig/SerDe/DcDeserializeCommon.inl"
#include "DataConfig/SerDe/DcSerDeUtils.h"
#include "UObject/Package.h"

void FDcTestLazyObjectSerDe::MakeFixture()
{
	UObject* TestsObject = DcSerDeUtils::FindFirstObject<UPackage>(TEXT("/Script/DataConfigTestsBefore59"), true);
	SoftField1 = TestsObject;
	SoftField2 = nullptr;
	LazyField1 = TestsObject;
	LazyField2 = nullptr;
}

static const TCHAR LazyObjectRefsJson[] = TEXT(R"(

	{
		"SoftField1" : "'/Script/DataConfigTestsBefore59'",
		"SoftField2" : null,
		"LazyField1" : "'/Script/DataConfigTestsBefore59'",
		"LazyField2" : null
	}

)");

DC_TEST("DataConfig.Core.Serialize.Before59.LazyObjectRefs")
{
	FDcTestLazyObjectSerDe Source;
	Source.MakeFixture();

	FDcJsonWriter Writer;
	UTEST_OK("Serialize LazyObjectRefs", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum(&Source)));
	Writer.Sb << TCHAR('\n');
	UTEST_EQUAL("Serialize LazyObjectRefs", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(LazyObjectRefsJson));

	return true;
}

DC_TEST("DataConfig.Core.Deserialize.Before59.LazyObjectRefs")
{
	FDcTestLazyObjectSerDe Dest;
	FDcJsonReader Reader(LazyObjectRefsJson);
	UTEST_OK("Deserialize LazyObjectRefs", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest)));

	FDcTestLazyObjectSerDe Expect;
	Expect.MakeFixture();
	UTEST_OK("Deserialize LazyObjectRefs", DcAutomationUtils::TestReadDatumEqual(FDcPropertyDatum(&Dest), FDcPropertyDatum(&Expect)));

	return true;
}

DC_TEST("DataConfig.Core.RoundTrip.Before59.LazyObjectRefs")
{
	FDcTestLazyObjectSerDe Source;
	Source.MakeFixture();
	FDcPropertyDatum SourceDatum(&Source);

	const TPair<EDcJsonSerializeType, EDcJsonDeserializeType> JsonModes[] = {
		{EDcJsonSerializeType::Default, EDcJsonDeserializeType::Default},
		{EDcJsonSerializeType::StringSoftLazy, EDcJsonDeserializeType::StringSoftLazy},
	};
	for (const auto& Mode : JsonModes)
	{
		FDcJsonWriter Writer;
		UTEST_OK("JSON LazyObjectRefs Roundtrip", DcAutomationUtils::SerializeInto(&Writer, SourceDatum, [&Mode](FDcSerializeContext& Ctx)
		{
			DcSetupJsonSerializeHandlers(*Ctx.Serializer, Mode.Key);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		FDcTestLazyObjectSerDe Dest;
		FDcJsonReader Reader(Writer.Sb.ToString());
		UTEST_OK("JSON LazyObjectRefs Roundtrip", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest), [&Mode](FDcDeserializeContext& Ctx)
		{
			DcSetupJsonDeserializeHandlers(*Ctx.Deserializer, Mode.Value);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_OK("JSON LazyObjectRefs Roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, FDcPropertyDatum(&Dest)));
	}

	const TPair<EDcMsgPackSerializeType, EDcMsgPackDeserializeType> MsgPackModes[] = {
		{EDcMsgPackSerializeType::Default, EDcMsgPackDeserializeType::Default},
		{EDcMsgPackSerializeType::StringSoftLazy, EDcMsgPackDeserializeType::StringSoftLazy},
		{EDcMsgPackSerializeType::InMemory, EDcMsgPackDeserializeType::InMemory},
	};
	for (const auto& Mode : MsgPackModes)
	{
		FDcMsgPackWriter Writer;
		UTEST_OK("MsgPack LazyObjectRefs Roundtrip", DcAutomationUtils::SerializeInto(&Writer, SourceDatum, [&Mode](FDcSerializeContext& Ctx)
		{
			DcSetupMsgPackSerializeHandlers(*Ctx.Serializer, Mode.Key);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		FDcTestLazyObjectSerDe Dest;
		FDcMsgPackReader Reader(FDcBlobViewData::From(Writer.GetMainBuffer()));
		UTEST_OK("MsgPack LazyObjectRefs Roundtrip", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(&Dest), [&Mode](FDcDeserializeContext& Ctx)
		{
			DcSetupMsgPackDeserializeHandlers(*Ctx.Deserializer, Mode.Value);
		}, DcAutomationUtils::EDefaultSetupType::SetupNothing));

		UTEST_OK("MsgPack LazyObjectRefs Roundtrip", DcAutomationUtils::TestReadDatumEqual(SourceDatum, FDcPropertyDatum(&Dest)));
	}

	return true;
}

DC_TEST("DataConfig.Core.Property.LazyObjectRefs")
{
	UDcTestBefore59Object* Object = NewObject<UDcTestBefore59Object>();
	UDcTestLazyObjectRefs* Src = NewObject<UDcTestLazyObjectRefs>();
	Src->LazyObjectField = Object;
	Src->LazyObjectArr[0] = Object;
	Src->LazyObjectArr[1] = NewObject<UDcTestBefore59Object>();
	Src->LazyObjectArr[2] = nullptr;
	Src->TestClassLazyObjectField = Object;

	UDcTestLazyObjectRefs* Dest = NewObject<UDcTestLazyObjectRefs>();
	UTEST_OK("LazyObjectRefs Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(Src),
		FDcPropertyDatum(Dest)
	));

	UTEST_TRUE("LazyObjectRefs Scalar", Dest->LazyObjectField.Get() == Object);
	UTEST_TRUE("LazyObjectRefs Array", Dest->LazyObjectArr[0].Get() == Object);
	UTEST_EQUAL("LazyObjectRefs Array", Dest->LazyObjectArr[1].Get(), Src->LazyObjectArr[1].Get());
	UTEST_TRUE("LazyObjectRefs Null", Dest->LazyObjectArr[2].IsNull());
	UTEST_EQUAL("LazyObjectRefs Typed", Dest->TestClassLazyObjectField.Get(), Object);

	Src->TestClassLazyObjectField = nullptr;
	UTEST_OK("LazyObjectRefs Null Roundtrip", DcPropertyPipeVisitAndTestEqual(
		FDcPropertyDatum(Src),
		FDcPropertyDatum(Dest)
	));
	UTEST_TRUE("LazyObjectRefs Typed Null", Dest->TestClassLazyObjectField.IsNull());

	return true;
}

DC_TEST("DataConfig.Core.Blurb.StringSoftLazy")
{
	UTEST_OK("Blurb StringSoftLazy", [&]{

		FDcTestLazyObjectSerDe Source{};
		UObject* TestsObject = StaticFindObject(UObject::StaticClass(), nullptr, TEXT("/Script/DataConfigTestsBefore59"));

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


static FDcResult _ReadLazyObjectReference(FDcDeserializeContext& Ctx, FObjectPropertyBase* /*ObjectProperty*/, UObject*& OutObject)
{
	// Let the lazy object handler check the referenced object's class.
	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));
	return DcSerDeUtils::TryStaticLocateObject(UObject::StaticClass(), *Value, OutObject);
}

DC_TEST("DataConfig.Core.Deserialize.Before59.DiagLazyObjectClassMismatch")
{
	UDcTestLazyObjectRefs* LazyDest = NewObject<UDcTestLazyObjectRefs>();
	FString Str = TEXT(R"(

		{
			"TestClassLazyObjectField" : "'/Script/DataConfigTestsBefore59'",
		}

	)");

	FDcJsonReader Reader(Str);
	UTEST_DIAG("DiagObjectClassMismatch", DcAutomationUtils::DeserializeFrom(&Reader, FDcPropertyDatum(LazyDest), [](FDcDeserializeContext& Ctx) {

		Ctx.Deserializer->AddDirectHandler(UClass::StaticClass(), FDcDeserializeDelegate::CreateStatic(DcCommonHandlers::HandlerMapToClassDeserialize));
		Ctx.Deserializer->AddDirectHandler(FLazyObjectProperty::StaticClass(), FDcDeserializeDelegate::CreateLambda([](FDcDeserializeContext& Ctx){
			return DcDeserializeLazyObjectReference<_ReadLazyObjectReference>(Ctx);
		}));

	}, DcAutomationUtils::EDefaultSetupType::SetupNothing), DcDSerDe, ClassLhsIsNotChildOfRhs);

	return true;
}
