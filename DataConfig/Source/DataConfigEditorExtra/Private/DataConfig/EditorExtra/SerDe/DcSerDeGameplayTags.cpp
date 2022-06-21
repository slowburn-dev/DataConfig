#include "DataConfig/EditorExtra/SerDe/DcSerDeGameplayTags.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Reader/DcReader.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Deserialize/DcDeserializeUtils.h"
#include "DataConfig/Extra/Misc/DcTestCommon.h"
#include "DataConfig/Json/DcJsonWriter.h"
#include "DataConfig/Serialize/DcSerializeUtils.h"

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
	return DcDeserializeUtils::PredicateIsUStruct<FGameplayTag>(Ctx);
}

FDcResult HandlerGameplayTagDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == nullptr
		|| Struct != FGameplayTag::StaticStruct())
	{
		return DC_FAIL(DcDSerDe, StructNotFound)
			<< TEXT("GameplayTag")
			<< Datum.Property.GetFName() << Datum.Property.GetClassName();
	}

	FGameplayTag* TagPtr = (FGameplayTag*)Datum.DataPtr;

	if (Next == EDcDataEntry::Nil)
	{
		DC_TRY(Ctx.Reader->ReadNil());
		*TagPtr = FGameplayTag::EmptyTag;

		return DcOk();
	}
	else if (Next == EDcDataEntry::String)
	{
		FString Str;
		DC_TRY(Ctx.Reader->ReadString(&Str));
		DC_TRY(_StringToGameplayTag(Ctx, Str, TagPtr));

		return DcOk();
	}
	else
	{
		return DC_FAIL(DcDSerDe, DataEntryMismatch2)
			<< EDcDataEntry::Nil << EDcDataEntry::String << Next;
	}
}

EDcDeserializePredicateResult PredicateIsGameplayTagContainer(FDcDeserializeContext& Ctx)
{
	return DcDeserializeUtils::PredicateIsUStruct<FGameplayTagContainer>(Ctx);
}

FDcResult HandlerGameplayTagContainerDeserialize(FDcDeserializeContext& Ctx)
{
	EDcDataEntry Next;
	DC_TRY(Ctx.Reader->PeekRead(&Next));

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == nullptr
		|| Struct != FGameplayTagContainer::StaticStruct())
	{
		return DC_FAIL(DcDSerDe, StructNotFound)
			<< TEXT("GameplayTagContainer")
			<< Datum.Property.GetFName() << Datum.Property.GetClassName();
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

	return DcOk();
}

EDcSerializePredicateResult PredicateIsGameplayTag(FDcSerializeContext& Ctx)
{
	return DcSerializeUtils::PredicateIsUStruct<FGameplayTag>(Ctx);
}

FDcResult HandlerGameplayTagSerialize(FDcSerializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == FGameplayTag::StaticStruct())
	{
		FGameplayTag* GameplayTagPtr = (FGameplayTag*)Datum.DataPtr;
		if (GameplayTagPtr->IsValid())
			DC_TRY(Ctx.Writer->WriteString(GameplayTagPtr->ToString()));
		else
			DC_TRY(Ctx.Writer->WriteNil());
	}
	else
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("GameplayTag") << Datum.Property.GetFName() << Datum.Property.GetClassName();
	}

	return DcOk();
}

EDcSerializePredicateResult PredicateIsGameplayTagContainer(FDcSerializeContext& Ctx)
{
	return DcSerializeUtils::PredicateIsUStruct<FGameplayTagContainer>(Ctx);
}

FDcResult HandlerGameplayTagContainerSerialize(FDcSerializeContext& Ctx)
{
	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Reader->ReadDataEntry(FStructProperty::StaticClass(), Datum));

	UScriptStruct* Struct = DcPropertyUtils::TryGetStructClass(Datum.Property);
	if (Struct == FGameplayTagContainer::StaticStruct())
	{
		DC_TRY(Ctx.Writer->WriteArrayRoot());
		FGameplayTagContainer* GameplayTagContainerPtr = (FGameplayTagContainer*)Datum.DataPtr;
		for (auto& Iter : *GameplayTagContainerPtr)
			DC_TRY(Ctx.Writer->WriteString(Iter.ToString()));
		DC_TRY(Ctx.Writer->WriteArrayEnd());
	}
	else
	{
		return DC_FAIL(DcDReadWrite, PropertyMismatch)
			<< TEXT("GameplayTagContainer") << Datum.Property.GetFName() << Datum.Property.GetClassName();
	}

	return DcOk();
}

} // namespace DcEditorExtra

DC_TEST("DataConfig.EditorExtra.GameplayTags")
{
	using namespace DcEditorExtra;
	FDcEditorExtraTestStructWithGameplayTag1 Dest;
	FDcPropertyDatum DestDatum(&Dest);

	UTEST_TRUE("Has natively added 'DataConfig.Foo.Bar'", UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")).IsValid());

	FString Str = TEXT(R"(
		{
			"TagField1" : null,
			"TagField2" : "DataConfig.Foo.Bar"
		}
	)");
	FDcJsonReader Reader(Str);

	{
		UTEST_OK("Editor Extra FGameplayTag SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTag),
				FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
			);
		}));

		UTEST_FALSE("Editor Extra FGameplayTag SerDe", Dest.TagField1.IsValid());
		UTEST_TRUE("Editor Extra FGameplayTag SerDe", Dest.TagField2.IsValid());
		UTEST_TRUE("Editor Extra FGameplayTag SerDe", Dest.TagField2 == UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")));
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Editor Extra FGameplayTag SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsGameplayTag),
				FDcSerializeDelegate::CreateStatic(HandlerGameplayTagSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Editor Extra FGameplayTag SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	FString StrBad = TEXT(R"(
		{
			"TagField1" : "DataConfig.Valid.But.Does.Not.Exist"
		}
	)");
	UTEST_OK("Editor Extra FGameplayTag SerDe", Reader.SetNewString(*StrBad));
	UTEST_DIAG("Editor Extra FGameplayTag SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
	[](FDcDeserializeContext& Ctx) {
		Ctx.Deserializer->AddPredicatedHandler(
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
	FDcPropertyDatum DestDatum(&Dest);

	UTEST_TRUE("Has natively added 'DataConfig.Foo.Bar'", UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar")).IsValid());

	FString Str = TEXT(R"(
		{
			"TagContainerField1" : [],
			"TagContainerField2" : [
				"DataConfig.Foo.Bar",
				"DataConfig.Foo.Bar.Baz",
				"DataConfig.Tar.Taz"
			]
		}
	)");

	{
		FDcJsonReader Reader(Str);
		UTEST_OK("Editor Extra FGameplayTagContainer SerDe", DcAutomationUtils::DeserializeFrom(&Reader, DestDatum,
		[](FDcDeserializeContext& Ctx) {
			Ctx.Deserializer->AddPredicatedHandler(
				FDcDeserializePredicate::CreateStatic(PredicateIsGameplayTagContainer),
				FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagContainerDeserialize)
			);
		}));

		UTEST_TRUE("Editor Extra FGameplayTagContainer SerDe", Dest.TagContainerField1.Num() == 0);
		UTEST_TRUE("Editor Extra FGameplayTagContainer SerDe", Dest.TagContainerField2.Num() == 3);
		UTEST_TRUE("Editor Extra FGameplayTagContainer SerDe", Dest.TagContainerField2.HasTagExact(
			UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar"))
		));
		UTEST_TRUE("Editor Extra FGameplayTagContainer SerDe", Dest.TagContainerField2.HasTagExact(
			UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"))
		));
		UTEST_TRUE("Editor Extra FGameplayTagContainer SerDe", Dest.TagContainerField2.HasTagExact(
			UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Tar.Taz"))
		));
	}

	{
		FDcJsonWriter Writer;
		UTEST_OK("Editor Extra FGameplayTagContainer SerDe", DcAutomationUtils::SerializeInto(&Writer, DestDatum,
		[](FDcSerializeContext& Ctx) {
			Ctx.Serializer->AddPredicatedHandler(
				FDcSerializePredicate::CreateStatic(PredicateIsGameplayTagContainer),
				FDcSerializeDelegate::CreateStatic(HandlerGameplayTagContainerSerialize)
			);
		}));
		Writer.Sb << TCHAR('\n');
		UTEST_EQUAL("Editor Extra FGameplayTagContainer SerDe", Writer.Sb.ToString(), DcAutomationUtils::DcReindentStringLiteral(Str));
	}

	return true;
}



