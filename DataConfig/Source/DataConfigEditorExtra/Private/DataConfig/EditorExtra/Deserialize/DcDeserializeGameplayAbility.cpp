#include "DataConfig/EditorExtra/Deserialize/DcDeserializeGameplayAbility.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "Abilities/GameplayAbility.h"
#include "MessageLogModule.h"
#include "Textures/SlateIcon.h"
#include "ToolMenuSection.h"
#include "Misc/FileHelper.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"

#include "DataConfig/DcTypes.h"
#include "DataConfig/DcEnv.h"
#include "DataConfig/Deserialize/DcDeserializer.h"
#include "DataConfig/Deserialize/DcDeserializerSetup.h"
#include "DataConfig/Diagnostic/DcDiagnosticSerDe.h"
#include "DataConfig/Diagnostic/DcDiagnosticReadWrite.h"
#include "DataConfig/EngineExtra/SerDe/DcSerDeGameplayTags.h"
#include "DataConfig/EditorExtra/Diagnostic/DcDiagnosticEditorExtra.h"
#include "DataConfig/Automation/DcAutomationUtils.h"
#include "DataConfig/Json/DcJsonReader.h"
#include "DataConfig/Property/DcPropertyWriter.h"
#include "DataConfig/Automation/DcAutomation.h"
#include "DataConfig/EngineExtra/SerDe/DcSerDeBlueprint.h"
#include "DataConfig/Extra/Types/DcPropertyPathAccess.h"
#include "DataConfig/SerDe/DcSerDeUtils.h"

namespace DcEditorExtra
{

namespace GameplayAbilityDetails
{

static TOptional<FDcDeserializer> GameplayAbilityDeserializer;

static void LazyInitializeDeserializer()
{
	using namespace DcEngineExtra;

	if (GameplayAbilityDeserializer.IsSet())
		return;

	GameplayAbilityDeserializer.Emplace();
	DcSetupJsonDeserializeHandlers(GameplayAbilityDeserializer.GetValue());

	GameplayAbilityDeserializer->FieldClassDeserializerMap[FClassProperty::StaticClass()] = FDcDeserializeDelegate::CreateStatic(HandlerBPClassReferenceDeserialize);

	GameplayAbilityDeserializer->AddStructHandler(
		FGameplayAttribute::StaticStruct(),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayAttributeDeserialize)
	);
	GameplayAbilityDeserializer->AddStructHandler(
		FGameplayTag::StaticStruct(),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagDeserialize)
	);
	GameplayAbilityDeserializer->AddStructHandler(
		FGameplayTagContainer::StaticStruct(),
		FDcDeserializeDelegate::CreateStatic(HandlerGameplayTagContainerDeserialize)
	);
}

} // namespace GameplayAbilityDetails



FDcResult HandlerGameplayAttributeDeserialize(FDcDeserializeContext& Ctx)
{
	FString AttributeStr;
	DC_TRY(Ctx.Reader->ReadString(&AttributeStr));

	int32 Ix;
	bool bFound = AttributeStr.FindChar(TCHAR('.'), Ix);
	if (!bFound)
		return DC_FAIL(DcDEditorExtra, InvalidGameplayAttribute) << AttributeStr;

	FStringView View =  AttributeStr;
	FString Head = FString(View.Left(Ix));
	FString Tail = FString(View.RightChop(Ix + 1));

	UClass* AttributeClass;
	DC_TRY(DcSerDeUtils::TryFindFirstObject<UClass>(*Head, false, AttributeClass));

	FProperty* AttributeProperty = DcPropertyUtils::FindEffectivePropertyByName(AttributeClass, *Tail);
	if (AttributeProperty == nullptr)
		return DC_FAIL(DcDReadWrite, CantFindPropertyByName) << Tail;

	FDcPropertyDatum Datum;
	DC_TRY(Ctx.Writer->WriteDataEntry(FStructProperty::StaticClass(), Datum));

	FGameplayAttribute* Attribute = (FGameplayAttribute*)Datum.DataPtr;
	Attribute->SetUProperty(AttributeProperty);

	return DcOk();
}

FDcResult DeserializeGameplayAbility(UGameplayAbility* Instance, FDcReader& Reader)
{
	using namespace GameplayAbilityDetails;

	LazyInitializeDeserializer();

	FDcPropertyWriter Writer(FDcPropertyDatum(UGameplayAbility::StaticClass(), Instance));

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &GameplayAbilityDeserializer.GetValue();
	DC_TRY(Ctx.Prepare());

	return GameplayAbilityDeserializer->Deserialize(Ctx);
}

FDcResult DeserializeGameplayEffect(UGameplayEffect* Instance, FDcReader& Reader)
{
	using namespace GameplayAbilityDetails;
	LazyInitializeDeserializer();

	FDcPropertyWriter Writer(FDcPropertyDatum(UGameplayEffect::StaticClass(), Instance));

	FDcDeserializeContext Ctx;
	Ctx.Reader = &Reader;
	Ctx.Writer = &Writer;
	Ctx.Deserializer = &GameplayAbilityDeserializer.GetValue();
	DC_TRY(Ctx.Prepare());

	return GameplayAbilityDeserializer->Deserialize(Ctx);
}

static FDcResult SelectJSONAndLoadIntoBlueprintCDO(FAssetData Asset, TFunctionRef<FDcResult(UBlueprint*, FDcReader& Reader)> DeserializeFunc)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
		return DcOk(); // silently fail on non desktop

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	TArray<FString> OpenFilenames;
	int32 FilterIndex = -1;
	bool bSelected = DesktopPlatform->OpenFileDialog(
		ParentWindowWindowHandle,
		TEXT("Select JSON File"),
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
		TEXT(""),
		TEXT("Json config file (*.json)|*.json"),
		EFileDialogFlags::None,
		OpenFilenames,
		FilterIndex
	);

	if (!bSelected)
		return DcOk();	// cancel is OK

	check(OpenFilenames.Num() == 1);
	FString Filename = OpenFilenames.Pop();

	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_IMPORT, FPaths::GetPath(Filename));

	FString JsonStr;
	bool bLoadFile = FFileHelper::LoadFileToString(JsonStr, *Filename, FFileHelper::EHashOptions::None);
	if (!bLoadFile)
		return DC_FAIL(DcDEditorExtra, LoadFileByPathFail) << Filename;

	FDcJsonReader Reader;
	DC_TRY(Reader.SetNewString(*JsonStr));
	Reader.DiagFilePath = MoveTemp(Filename);

	UBlueprint* Blueprint = CastChecked<UBlueprint>(Asset.GetAsset());

	//  regenerate CDO before deserializing from JSON to avoid stale values from last run
	Blueprint->GeneratedClass->PurgeClass(false);
	FCompilerResultsLog Results;
	FKismetCompilerOptions CompileOptions;
	FKismetCompilerContext Compiler(Blueprint, Results, CompileOptions);
	Compiler.Compile();

	if (Results.NumErrors > 0)
		return DC_FAIL(DcDEditorExtra, KismetCompileFail) << Blueprint->GetFriendlyName();

	DC_TRY(DeserializeFunc(Blueprint, Reader));

	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	return DcOk();
}

TSharedRef<FExtender> GameplayAbilityEffectExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	for (auto& Asset : SelectedAssets)
	{
		FString ParentClassPath;
		if(!Asset.GetTagValue(FBlueprintTags::NativeParentClassPath, ParentClassPath))
			continue;

		UObject* Outer = nullptr;
		ResolveName( Outer, ParentClassPath, false, false );
		UClass* NativeParentClass = DcSerDeUtils::FindFirstObject<UClass>(*ParentClassPath, false);

		if (!NativeParentClass)
			continue;

		if (NativeParentClass->IsChildOf(UGameplayAbility::StaticClass()))
		{
			Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateLambda([Asset](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadAbilityFromJson", "DataConfig Load Ability From JSON"),
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadAbilityFromJsonTooltip", "DataConfig load ability default values from a JSON file"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([Asset]{

								FDcResult Ret = SelectJSONAndLoadIntoBlueprintCDO(Asset, [](UBlueprint* Blueprint, FDcReader& Reader)
								{
									UGameplayAbility* AbilityCDO = GetMutableDefault<UGameplayAbility>(Blueprint->GeneratedClass);
									return DeserializeGameplayAbility(AbilityCDO, Reader);
								});

								if (!Ret.Ok())
								{
									DcEnv().FlushDiags();

									FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
									MessageLogModule.OpenMessageLog(TEXT("DataConfig"));
								}
							}),
							FCanExecuteAction()
						)
					);
			}));

			break;
		}
		else if (NativeParentClass->IsChildOf(UGameplayEffect::StaticClass()))
		{
			Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateLambda([Asset](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadEffectFromJson", "DataConfig Load Effect From JSON"),
						NSLOCTEXT("DataConfigEditorExtra", "DcEditorExtra_LoadEffectFromJsonTooltip", "DataConfig load effect default values from a JSON file"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([Asset]{

								FDcResult Ret = SelectJSONAndLoadIntoBlueprintCDO(Asset, [](UBlueprint* Blueprint, FDcReader& Reader)
								{
									UGameplayEffect* EffectCDO = GetMutableDefault<UGameplayEffect>(Blueprint->GeneratedClass);
									return DeserializeGameplayEffect(EffectCDO, Reader);
								});

								if (!Ret.Ok())
								{
									DcEnv().FlushDiags();

									FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
									MessageLogModule.OpenMessageLog(TEXT("DataConfig"));
								}
							}),
							FCanExecuteAction()
						)
					);
			}));

			break;
		}
	}

	return Extender;
}

} // namespace DcEditorExtra


DC_TEST("DataConfig.EditorExtra.GameplayAbility")
{
	FString Str = TEXT(R"(
		{
			/// Tags
			"AbilityTags" : [
				"DataConfig.Foo.Bar",
				"DataConfig.Foo.Bar.Baz",
			],
			"CancelAbilitiesWithTag" : [
				"DataConfig.Foo.Bar.Baz",
				"DataConfig.Tar.Taz",
			],
			/// Costs
			"CostGameplayEffectClass" : "/DataConfig/DcFixture/DcTestGameplayEffectAlpha",
			/// Advanced
			"ReplicationPolicy" : "ReplicateYes",
			"InstancingPolicy" : "InstancedPerActor",
		}
	)");
	FDcJsonReader Reader(Str);

	UGameplayAbility* TmpAbility = NewObject<UGameplayAbility>();
	UTEST_OK("Editor Extra UGameplayAbility Deserialize", DcEditorExtra::DeserializeGameplayAbility(TmpAbility, Reader));

	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", TmpAbility->GetReplicationPolicy() == EGameplayAbilityReplicationPolicy::ReplicateYes);
	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", TmpAbility->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor);

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	auto& AbilityTags = TmpAbility->AbilityTags;
#else
	auto& AbilityTags = TmpAbility->GetAssetTags();
#endif // !UE_VERSION_OLDER_THAN(5, 5, 0)

	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", AbilityTags.HasTagExact(
		UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar"))
	));
	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", AbilityTags.HasTagExact(
		UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"))
	));

	//	use `DcExtra::GetDatumPropertyByPath` to get protected properties here
	FGameplayTagContainer* ContainerPtr = DcExtra::GetDatumPropertyByPath<FGameplayTagContainer>(FDcPropertyDatum(TmpAbility), TEXT("CancelAbilitiesWithTag"));
	UTEST_NOT_NULL("Editor Extra UGameplayAbility Deserialize", ContainerPtr);
	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", ContainerPtr->HasTagExact(
			UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"))
	));
	UTEST_TRUE("Editor Extra UGameplayAbility Deserialize", ContainerPtr->HasTagExact(
			UGameplayTagsManager::Get().RequestGameplayTag(TEXT("DataConfig.Tar.Taz"))
	));

	return true;
}

DC_TEST("DataConfig.EditorExtra.GameplayEffect")
{
	FString Str = TEXT(R"(
		{
			/// Effect
			"DurationPolicy" : "Infinite",
			"Modifiers" : [
				{
					"Attribute" : "DcTestAttributeSet.Mana",
				},
				{
					"Attribute" : "DcTestAttributeSet.Health",
				},
			],
		}
	)");
	FDcJsonReader Reader(Str);

	UGameplayEffect* TmpEffect = NewObject<UGameplayEffect>();
	UTEST_OK("Editor Extra UGameplayEffect Deserialize", DcEditorExtra::DeserializeGameplayEffect(TmpEffect, Reader));

	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->DurationPolicy == EGameplayEffectDurationType::Infinite);
	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->Modifiers.Num() == 2);

	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->Modifiers[0].Attribute.AttributeName == TEXT("Mana"));
	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->Modifiers[0].Attribute.GetAttributeSetClass() == UDcTestAttributeSet::StaticClass());

	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->Modifiers[1].Attribute.AttributeName == TEXT("Health"));
	UTEST_TRUE("Editor Extra UGameplayEffect Deserialize", TmpEffect->Modifiers[1].Attribute.GetAttributeSetClass() == UDcTestAttributeSet::StaticClass());

	return true;
}



