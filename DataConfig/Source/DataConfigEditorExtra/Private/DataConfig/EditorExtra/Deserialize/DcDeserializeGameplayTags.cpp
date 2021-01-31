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

static FDcResult _StringToGameplayTag(FDcDeserializeContext& Ctx, const FString& Str, FGameplayTag* OutTagPtr)
{
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

	*OutTagPtr = Tag;
	return DcOk();
}

EDcDeserializePredicateResult PredicateIsGameplayTag(FDcDeserializeContext& Ctx)
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
		DC_TRY(_StringToGameplayTag(Ctx, Str, TagPtr));
	}
	else
	{
		return DcNoEntry();
	}

	return DcOkWithProcessed(OutRet);
}

EDcDeserializePredicateResult PredicateIsGameplayTagContainer(FDcDeserializeContext& Ctx)
{
	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Ctx.TopProperty());
	return Struct && Struct == FGameplayTagContainer::StaticStruct()
		? EDcDeserializePredicateResult::Process
		: EDcDeserializePredicateResult::Pass;
}

FDcResult HandlerGameplayTagContainerDeserialize(FDcDeserializeContext& Ctx, EDcDeserializeResult& OutRet)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));
	bool bReadPass = Next == EDcDataEntry::ArrayRoot;

	bool bWritePass;
	DC_TRY(Ctx.Writer->PeekWrite(EDcDataEntry::StructRoot, &bWritePass));

	if (!bReadPass && bWritePass)
		return DcOkWithFallThrough(OutRet);

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == nullptr
		|| Struct != FGameplayTagContainer::StaticStruct())
	{
		return DC_FAIL(DcDDeserialize, StructNotFound)
			<< TEXT("GameplayTagContainer") << Datum.Property;
	}

	FGameplayTagContainer* ContainerPtr = (FGameplayTagContainer*)Datum.DataPtr;

	DC_TRY(Ctx.Reader->ReadArrayRoot());

	EDcDataEntry CurPeek;
	while (true)
	{
		DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
		if (CurPeek == EDcDataEntry::ArrayEnd)
			break;

		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));

		FGameplayTag Tag;
		DC_TRY(_StringToGameplayTag(Ctx, Str, &Tag));

		ContainerPtr->AddTag(Tag);
	}

	DC_TRY(Ctx.Reader->ReadArrayEnd());

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

	UTEST_FALSE("Editor Extra FGameplayTag Deserialize", Dest.TagField1.IsValid());
	UTEST_TRUE("Editor Extra FGameplayTag Deserialize", Dest.TagField2.IsValid());
	UTEST_TRUE("Editor Extra FGameplayTag Deserialize", Dest.TagField2 == UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")));

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

DC_TEST("DataConfig.EditorExtra.GameplayTagContainer")
{
	using namespace DcEditorExtra;
	FDcEditorExtraTestStructWithGameplayTag2 Dest;
	FDcPropertyDatum DestDatum(FDcEditorExtraTestStructWithGameplayTag2::StaticStruct(), &Dest);

	UTEST_TRUE("Has natively added 'DataConfig.Foo.Bar'", UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")).IsValid());

	FString Str = TEXT(R"(
		{
			"TagContainerField1" : [],
			"TagContainerField2" : [
				"DataConfig.Foo.Bar",
				"DataConfig.Foo.Bar.Baz",
				"DataConfig.Tar.Taz",
			]
		}
	)");
	FDcJsonReader Reader(Str);

	UTEST_OK("Editor Extra FGameplayTagContainer Deserialize", DcAutomationUtils::DeserializeJsonInto(&Reader, DestDatum,
	[](FDcDeserializer& Deserializer, FDcDeserializeContext& Ctx) {
		Deserializer.AddPredicatedHandler(
			FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTagContainer),
			FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagContainerDeserialize)
		);
	}));

	UTEST_TRUE("Editor Extra FGameplayTagContainer Deserialize", Dest.TagContainerField1.Num() == 0);
	UTEST_TRUE("Editor Extra FGameplayTagContainer Deserialize", Dest.TagContainerField2.Num() == 3);
	UTEST_TRUE("Editor Extra FGameplayTagContainer Deserialize", Dest.TagContainerField2.HasTagExact(
		UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar"))
	));
	UTEST_TRUE("Editor Extra FGameplayTagContainer Deserialize", Dest.TagContainerField2.HasTagExact(
		UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"))
	));
	UTEST_TRUE("Editor Extra FGameplayTagContainer Deserialize", Dest.TagContainerField2.HasTagExact(
		UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Tar.Taz"))
	));

	return true;
}



