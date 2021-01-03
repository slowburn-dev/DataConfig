#include "DataConfig/Deserialize/Handlers/Json/DcJsonPrimitiveDeserializers.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"

namespace DcJsonHandlers {

EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx)
{
	return Ctx.TopProperty().IsA<FNumericProperty>()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FNumericProperty>())
		return DcOkWithFallThrough(OutRet);

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (!DcTypeUtils::IsNumericDataEntry(Next))
		return DC_FAIL(DcDDeserialize, ExpectNumericEntry) << Next;

	//	property writer driven coercion
	Next = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());
	if (!Ctx.Reader->Coercion(Next))
		return DC_FAIL(DcDDeserialize, CoercionFail) << Next;;

	DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer);

	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FBoolProperty>())
	{
		return DcOkWithFallThrough(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (Next != EDcDataEntry::Bool)
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::Bool << Next;
	}

	bool Value;
	DC_TRY(Ctx.Reader->ReadBool(&Value));
	DC_TRY(Ctx.Writer->WriteBool(Value));

	OutRet = EDcDeserializeResult::Success;
	return DcOkWithProcessed(OutRet);
}

FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FNameProperty>())
	{
		return DcOkWithFallThrough(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FStrProperty>())
	{
		return DcOkWithFallThrough(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteString(Value));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FTextProperty>())
	{
		return DcOkWithFallThrough(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteText(FText::FromString(MoveTemp(Value))));
		return DcOkWithProcessed(OutRet);
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

FDcResult HandlerEnumDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	if (!Ctx.TopProperty().IsA<FEnumProperty>())
	{
		return DcOkWithFallThrough(OutRet);
	}

	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next != EDcDataEntry::String)
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}

	FEnumProperty* EnumProperty = CastFieldChecked<FEnumProperty>(Ctx.TopProperty().ToFieldUnsafe());
	UEnum* EnumClass = EnumProperty->GetEnum();

	FString Value;
	DC_TRY(Ctx.Reader->ReadString(&Value));

	FName ValueName(EnumClass->GenerateFullEnumName(*Value));
	if (!EnumClass->IsValidEnumName(ValueName))
		return DC_FAIL(DcDDeserialize, EnumValueNotFound) << EnumClass->GetFName() << Value;

	FDcEnumData EnumData;
	EnumData.Signed64 = EnumClass->GetValueByName(ValueName);

	return Ctx.Writer->WriteEnum(EnumData);
}

EDcDeserializePredicateResult PredicateIsEnumFlagsProperty(FDcDeserializeContext& Ctx)
{
	FEnumProperty* EnumProperty = DcPropertyUtils::CastFieldVariant<FEnumProperty>(Ctx.TopProperty());
	if (EnumProperty == nullptr)
		return EDcDeserializePredicateResult::Pass;

	UEnum* EnumClass = EnumProperty->GetEnum();
#if WITH_METADATA
	return EnumClass->HasMetaData(TEXT("Bitflags"))
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
#else
	//	Program target is missing `UEnum::HasMetaData`
	return ((UField*)EnumClass)->HasMetaData(TEXT("Bitflags"))
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
#endif
}

FDcResult HandleEnumFlagsDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::ArrayRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::Enum, &bWritePass));

	if (!(bReadPass && bWritePass))
		return DcOkWithFallThrough(OutRet);

	FEnumProperty* EnumProperty = CastFieldChecked<FEnumProperty>(Ctx.TopProperty().ToFieldUnsafe());
	UEnum* EnumClass = EnumProperty->GetEnum();

	FDcEnumData EnumData;
	EnumData.Signed64 = 0;

	DC_TRY(Ctx.Reader->ReadArrayRoot());
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&Next));
		if (Next == EDcDataEntry::ArrayEnd)
			break;

		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		FName ValueName(EnumClass->GenerateFullEnumName(*Value));
		if (!EnumClass->IsValidEnumName(ValueName))
			return DC_FAIL(DcDDeserialize, EnumValueNotFound) << EnumClass->GetFName() << Value;

		EnumData.Signed64 |= EnumClass->GetValueByName(ValueName);
	}
	DC_TRY(Ctx.Reader->ReadArrayEnd());

	return Ctx.Writer->WriteEnum(EnumData);
}

}	// namespace DcHandlers

