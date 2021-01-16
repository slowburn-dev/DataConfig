#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"

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
	DC_TRY(Ctx.Writer->WriteStructRoot(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

	DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

	DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

	DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

	DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
	DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

	DC_TRY(Ctx.Writer->WriteStructEnd(FDcStructStat{ TEXT("Color"), FDcStructStat::WriteCheckName }));

	return DcOk();
}

template<EDcColorDeserializeMethod Method>
FDcResult TemplatedHandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::String;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FString ColorStr;
	DC_TRY(Ctx.Reader->ReadString(&ColorStr));

	FColor Color = FColor::FromHex(ColorStr);

	DC_TRY(TemplatedWriteColorDispatch<Method>(Color, Ctx));
	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	return TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriteBlob>(Ctx, OutRet);
}

} // namespace DcExtra

DC_TEST("DataConfig.Extra.Deserialize.Color")
{
	using namespace DcExtra;
	FDcExtraTestStructWithColor1 Dest;
	FDcPropertyDatum DestDatum(FDcExtraTestStructWithColor1::StaticStruct(), &Dest);

	FString Str = TEXT(R"(
		{
			"ColorField1" : "#0000FFFF",
			"ColorField2" : "#FF0000FF",
		}
	)");
	FDcJsonReader Reader;

	FDcExtraTestStructWithColor1 Expect;

	Expect.ColorField1 = FColor::Blue;
	Expect.ColorField2 = FColor::Red;

	FDcPropertyDatum ExpectDatum(FDcExtraTestStructWithColor1::StaticStruct(), &Expect);

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor Deserialize", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WritePointer>)
			);
		}));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor Deserialize", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriteDataEntry>)
			);
		}));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor Deserialize", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriteBlob>)
			);
		}));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	{
		Dest = FDcExtraTestStructWithColor1{};
		UTEST_OK("Extra FColor Deserialize", Reader.SetNewString(*Str));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
		[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
			Deserializer.AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
				FDcDeserializeDelegate::CreateStatic(TemplatedHandlerColorDeserialize<EDcColorDeserializeMethod::WriterAPI>)
			);
		}));
		UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));
	}

	return true;
}

