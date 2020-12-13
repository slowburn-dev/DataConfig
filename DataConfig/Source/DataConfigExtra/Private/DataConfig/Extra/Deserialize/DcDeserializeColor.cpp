#include "DataConfig/Extra/Deserialize/DcDeserializeColor.h"
#include "DataConfig/DcTypes.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Writer/DcWriter.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/DcEnv.h"

namespace DcExtra
{

EDcDeserializePredicateResult DATACONFIGEXTRA_API PredicateIsColorStruct(FDcDeserializeContext& Ctx)
{
	//	TODO DcDeserializeUtil this
	if (FStructProperty* StructProperty = DcPropertyUtils::CastFieldVariant<FStructProperty>(Ctx.TopProperty()))
	{
		return StructProperty->Struct->GetFName() == TEXT("Color")
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else if (UScriptStruct* StructClass = DcPropertyUtils::CastFieldVariant<UScriptStruct>(Ctx.TopProperty()))
	{
		return StructClass->GetFName() == TEXT("Color")
			? EDcDeserializePredicateResult::Process
			: EDcDeserializePredicateResult::Pass;
	}
	else
	{
		return EDcDeserializePredicateResult::Pass;
	}
}

FDcResult DATACONFIGEXTRA_API HandlerColorDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::String;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithCanNotProcess(OutRet);

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
		DC_TRY(Ctx.Writer->WriteStructRoot(TEXT("Color")));

		DC_TRY(Ctx.Writer->WriteName(TEXT("B")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.B));

		DC_TRY(Ctx.Writer->WriteName(TEXT("G")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.G));

		DC_TRY(Ctx.Writer->WriteName(TEXT("R")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.R));

		DC_TRY(Ctx.Writer->WriteName(TEXT("A")));
		DC_TRY(Ctx.Writer->WriteUInt8(Color.A));

		DC_TRY(Ctx.Writer->WriteStructEnd(TEXT("Color")));
	}
	else
	{
		return DcNoEntry();
	}


	return DcOk();
}

}	//	namespace DcExtra


