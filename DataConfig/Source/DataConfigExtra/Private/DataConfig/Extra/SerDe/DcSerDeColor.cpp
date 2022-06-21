#include "DataConfig/Extra/SerDe/DcSerDeColor.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"

namespace DcExtra
{

EDcDeserializePredicateResult PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
    UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
    return Struct && Struct == TBaseStructure<FColor>::Get()
        ? EDcDeserializePredicateResult::Process
        : EDcDeserializePredicateResult::Pass;

}

///	Showcasing different ways of deserializing into `FColor`
///	End results should be identical
enum class EDcColorDeserializeMethod
{
	WritePointer,
	WriteDataEntry,
	WriteBlob,
	WriterAPI,
};

template<EDcColorDeserializeMethod Method>
FDcResult TemplatedWriteColorDispatch(const FColor& Color, FDcDeserializeContext& Ctx)
{
	return DcNoEntry();
}

template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WritePointer>(const FColor& Color, FDcDeserializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FColor* ColorPtr = (FColor*)Datum.DataPtr;
	*ColorPtr = Color;

	return DcOk();
}

template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriteDataEntry>(const FColor& Color, FDcDeserializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	Datum.CastFieldChecked<FStructProperty>()->CopySingleValue(Datum.DataPtr, &Color);
	return DcOk();
}

template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriteBlob>(const FColor& Color, FDcDeserializeContext& Ctx)
{
	return Ctx.Writer->WriteBlob({
		(uint8*)&Color,
		sizeof(FColor)
	});
}

template<>
FDcResult TemplatedWriteColorDispatch<EDcColorDeserializeMethod::WriterAPI>(const FColor& Color, FDcDeserializeContext& Ctx)
{
	FDcStructAccess Access{ FDcStructAccess::WriteCheckName, TEXT("Color")  };
	DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

	DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

	DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

	DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

	DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

	DC_TRY(Ctx.Writer->WriteStructEndAccess(Access));

	return DcOk();
}

template<EDcColorDeserializeMethod Method>
FDcResult TemplatedHandlerColorDeserialize(FDcDeserializeContext& Ctx)
{
	FString ColorStr;
	DC_TRY(Ctx.Reader->ReadString(&ColorStr));

	FColor Color = FColor::FromHex(ColorStr);

	DC_TRY(TemplatedWriteColorDispatch<Method>(Color, Ctx));
	return DcOk();
}

EDcSerializePredicateResult PredicateIsColorStruct(FDcSerializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == TBaseStructure<FColor>::Get()
		? EDcSerializePredicateResult::Process
		: EDcSerializePredicateResult::Pass;

}

FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FString ColorStr;
	DC_TRY(Ctx.Reader->ReadString(&ColorStr));
	FColor Color = FColor::FromHex(ColorStr);

	FColor* ColorPtr = (FColor*)Datum.DataPtr;
	*ColorPtr = Color;

	return DcOk();
}

FDcResult HandlerColorSerialize(FDcSerializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	FColor* ColorPtr = (FColor*)Datum.DataPtr;
	DC_TRY(Ctx.Writer->WriteString(TEXT("#") + ColorPtr->ToHex()));

	return DcOk();
}

} // namespace DcExtra

DC_TEST("DataConfig.Extra.SerDe.Color")
{
	using namespace DcExtra;

	FDcExtraTestStructWithColor1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	FString Str = TEXT(R"(
		{
			"ColorField1" : "#0000FFFF",
			"ColorField2" : "#FF0000FF"
		}
	)");
	FDcJsonReader Reader;

	FDcExtraTestStructWithColor1 Expect;

	Expect.ColorField1 = FColor::Blue;
	Expect.ColorField2 = FColor::Red;

	FDcPropertyDatum ExpectDatum(&Expect);

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor SerDe", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WritePointer>)
			);
		}));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor SerDe", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriteDataEntry>)
			);
		}));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor SerDe", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriteBlob>)
			);
		}));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor SerDe", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriterAPI>)
			);
		}));
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Extra FColor SerDe", DcAutomationUtils::SerializeInto(&Writer, FDcPropertyDatum{&Expect},
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcSerializeDelegate::CreateStatic(HandlerColorSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');

		UTEST_EQUAL("Extra FColor SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str))
	}

	return true;
}

