#include "DataConfig/Deserialize/Handlers/Json/DcJsonPrimitiveDeserializers.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "UObject/TextProperty.h"

namespace DcJsonHandlers {

EDcDeserializePredicateResult PredicateIsNumericProperty(FDcDeserializeContext& Ctx)
{
	return Ctx.TopProperty().IsA<FNumericProperty>()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerNumericDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	if (!DcTypeUtils::IsNumericDataEntry(Next))
		return DC_FAIL(DcDDeserialize, ExpectNumericEntry) << Next;

	//	property writer driven coercion
	Next = DcPropertyUtils::PropertyToDataEntry(Ctx.TopProperty());
	if (!Ctx.Reader->Coercion(Next))
		return DC_FAIL(DcDDeserialize, CoercionFail) << Next;;

	DC_TRY(DcDeserializeUtils::DispatchPipeVisit(Next, Ctx.Reader, Ctx.Writer));
	return DcOk();
}

FDcResult HandlerBoolDeserialize(FDcDeserializeContext& Ctx)
{
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

	return DcOk();
}

FDcResult HandlerNameDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteName(FName(*Value)));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

FDcResult HandlerStringDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteString(Value));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

FDcResult HandlerTextDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	if (Next == EDcDataEntry::String)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));
		DC_TRY(Ctx.Writer->WriteText(FText::FromString(MoveTemp(Value))));
		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDDeserialize, DataEntryMismatch)
			<< EDcDataEntry::String << Next;
	}
}

EDcDeserializePredicateResult PredicateIsEnumProperty(FDcDeserializeContext& Ctx)
{
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;
	bool bFoundEnum = DcPropertyUtils::TryGetEnumPropertyOut(Ctx.TopProperty(), Enum, UnderlyingProperty);
	
	return bFoundEnum
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerEnumDeserialize(FDcDeserializeContext& Ctx)
{
	UEnum* Enum = nullptr;
	FNumericProperty* UnderlyingProperty = nullptr;

	DC_TRY(DcPropertyUtils::GetEnumProperty(Ctx.TopProperty(), Enum, UnderlyingProperty));

	bool bIsBitFlags;
#if WITH_METADATA
	bIsBitFlags = Enum->HasMetaData(TEXT("Bitflags"));
#else
	//	Program target is missing `UEnum::HasMetaData`
	bIsBitFlags = ((UField*)Enum)->HasMetaData(TEXT("Bitflags"));
#endif
	
	if (!bIsBitFlags)
	{
		FString Value;
		DC_TRY(Ctx.Reader->ReadString(&Value));

		FName ValueName(Enum->GenerateFullEnumName(*Value));
		if (!Enum->IsValidEnumName(ValueName))
			return DC_FAIL(DcDDeserialize, EnumNameNotFound) << Enum->GetFName() << Value;

		FDcEnumData EnumData;
		EnumData.Signed64 = Enum->GetValueByName(ValueName);

		DC_TRY(Ctx.Writer->WriteEnum(EnumData));
		return DcOk();
	}
	else
	{
		FDcEnumData EnumData;
		EnumData.Signed64 = 0;

		DC_TRY(Ctx.Reader->ReadArrayRoot());
		while (true)
		{
			EDcDataEntry Next;
			DC_TRY(Ctx.Reader->PeekRead(&Next));
			if (Next == EDcDataEntry::ArrayEnd)
				break;

			FString Value;
			DC_TRY(Ctx.Reader->ReadString(&Value));

			FName ValueName(Enum->GenerateFullEnumName(*Value));
			if (!Enum->IsValidEnumName(ValueName))
				return DC_FAIL(DcDDeserialize, EnumNameNotFound) << Enum->GetFName() << Value;

			EnumData.Signed64 |= Enum->GetValueByName(ValueName);
		}
		DC_TRY(Ctx.Reader->ReadArrayEnd());
		DC_TRY(Ctx.Writer->WriteEnum(EnumData));

		return DcOk();
	}
}

}	// namespace DcJsonHandlers

