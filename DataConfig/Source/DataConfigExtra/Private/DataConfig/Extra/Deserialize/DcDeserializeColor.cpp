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
	return Struct && Struct->GetFName() == TEXT("Color")
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
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

	enum class EDeserializeType
	{
		WriteDataEntry,
		WriteBlob,
		WriterAPI,
	};
	const EDeserializeType Type = EDeserializeType::WriteBlob;

	if (Type == EDeserializeType::WriteDataEntry)
	{
		FDcPropertyDatum Datum;
		DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

		Datum.CastFieldChecked<FStructProperty>()->CopySingleValue(Datum.DataPtr, &Color);
	}
	else if (Type == EDeserializeType::WriteBlob)
	{
		DC_TRY(Ctx.Writer->WriteBlob({
			(uint8*)&Color,
			sizeof(FColor)
		}));
	}
	else if (Type == EDeserializeType::WriterAPI)
	{
		DC_TRY(Ctx.Writer->WriteStructRoot(FDcStructStat{TEXT("Color"), FDcStructStat::WriteCheckName}));

		DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

		DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

		DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

		DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

		DC_TRY(Ctx.Writer->WriteStructEnd(FDcStructStat{TEXT("Color"), FDcStructStat::WriteCheckName}));
	}
	else
	{
		return DcNoEntry();
	}


	return DcOk();
}

}	//	namespace DcExtra

DC_TEST("DataConfig.Extra.Deserialize.Color")
{
	using namespace DcExtra;
	FDcExtraTestStructWithColor1 Dest;
	FDcPropertyDatum DestDatum(FDcExtraTestStructWithColor1::StaticStruct(), &Dest);

	FDcJsonReader Reader;
	FString Str = TEXT(R"(
		{
			"ColorField1" : "#0000FFFF",
			"ColorField2" : "#FF0000FF",
		}
	)");
	Reader.SetNewString(*Str);

	FDcExtraTestStructWithColor1 Expect;

	Expect.ColorField1 = FColor::Blue;
	Expect.ColorField2 = FColor::Red;

	FDcPropertyDatum ExpectDatum(FDcExtraTestStructWithColor1::StaticStruct(), &Expect);

	UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsColorStruct),
			FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize)
		);
	}));
	UTEST_OK("Extra FColor Deserialize", DcAutomationUtils::TestReadDatumEqual(DestDatum, ExpectDatum));

	return true;
}

