#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayTags.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticDeserialize.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"

namespace DcEditorExtra {

EDcDeserializePredicateResult DcEditorExtra::PredicateIsGameplayTag(FDcDeserializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == FGameplayTag::StaticStruct()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerGameplayTagDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::String
		|| Next == EDcDataEntry::Nil;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!bReadPass && bWritePass)
		return DcOkWithFallThrough(OutRet);

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == nullptr
		|| Struct != FGameplayTag::StaticStruct())
	{
		return DC_FAIL(DcDDeserialize, StructNotFound)
			<< TEXT("GameplayTag") << Datum.Property;
	}

	FGameplayTag* TagPtr = (FGameplayTag*)Datum.DataPtr;

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		*TagPtr = FGameplayTag::EmptyTag;
	}
	else if (Next == EDcDataEntry::String)
	{
		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));

		FString FixedString;
		FText Err;
		if (!FGameplayTag::IsValidGameplayTagString(Str, &Err, &FixedString))
		{
			return DC_FAIL(DcDEditorExtra, InvalidGameplayTagStringFixErr)
				<< Str << FixedString << Err;
		}

		FName TagName(*Str);
		FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
		if (!Tag.IsValid())
		{
			return DC_FAIL(DcDEditorExtra, InvalidGameplayTagString) << Str;
		}

		*TagPtr = Tag;
	}
	else
	{
		return DcNoEntry();
	}

	return DcOkWithProcessed(OutRet);
}

} // namespace DcEditorExtra



DC_TEST("DataConfig.EditorExtra.GameplayTags")
{
	using namespace DcEditorExtra;
	FDcEditorExtraTestStructWithGameplayTag1 Dest;
	FDcPropertyDatum DestDatum(FDcEditorExtraTestStructWithGameplayTag1::StaticStruct(), &Dest);

	UTEST_TRUE("Has natively added 'DataConfig.Foo.Bar'", UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")).IsValid());

	FString Str = TEXT(R"(
		{
			"TagField1" : null,
			"TagField2" : "DataConfig.Foo.Bar"
		}
	)");
	FDcJsonReader Reader(Str);

	FDcEditorExtraTestStructWithGameplayTag1 Expect;
	FDcPropertyDatum ExpectDatum(FDcEditorExtraTestStructWithGameplayTag1::StaticStruct(), &Expect);

	UTEST_OK("Editor Extra FGameplayTag Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTag),
			FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
		);
	}));


	FString StrBad = TEXT(R"(
		{
			"TagField1" : "DataConfig.Valid.But.Does.Not.Exist"
		}
	)");
	UTEST_OK("Editor Extra FGameplayTag Deserialize", Reader.SetNewString(*StrBad));
	UTEST_DIAG("Editor Extra FGameplayTag Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTag),
			FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
		);
	}), DcDEditorExtra, InvalidGameplayTagString);

	return true;
}




